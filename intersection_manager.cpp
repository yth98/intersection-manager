# Algorithms, NTUEE (Fall 2017)
# Intersection Manager

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

enum Direction {
   DIR_N = 0,
   DIR_E = 1,
   DIR_S = 2,
   DIR_W = 3,
   EMPTY = 4,

   UNDEF
};

class treeNode {
public:
   treeNode() :_depth(0),_late(0) {}
   treeNode(unsigned d) :_depth(d) {}
   ~treeNode() {
      for (size_t i = 0; i < _child.size(); i++) delete _child[i];
   }

   void setGo (bool* go) { for (size_t i = 0; i < 4; i++) _go[i] = go[i]; }
   void setLate (unsigned late) { _late = late; }
   void appendChild (treeNode* child) { _child.push_back(child); }

   vector<treeNode*>  _child;
   unsigned _depth; // tree level, go through how many rounds
   bool               _go[4]; // which cars are chosen to reach this node
   unsigned           _late;  // latency added
};

// declarations
void explore (treeNode*);
void printOut(FILE*, vector<Direction>*);

// global variables
unsigned lanePtr[4] = {0};
long long unsigned heu_b;        // boundary constant
long long unsigned node_cnt = 0;    // total explored nodes
vector<Direction> carFlow[4];    // input  (N,E,S,W)
vector<Direction> carArrange[4]; // output

int main (int argc, char** argv) {
   FILE *input, *output;
   char buffer[4096];

   if (argc < 3) { // filename is not given
      printf("USAGE: car.exec <input file name> <output file name>\n");
      return 0;
   }
   input = fopen(argv[1], "r");
   if (!input) { // fail to open the file
      fprintf(stderr, "[ERROR] Unable to open input file!\n");
      return -1;
   }
   output = fopen(argv[2], "w");
   if (!output) { // fail to open the file
      fprintf(stderr, "[ERROR] Unable to create output file!\n");
      return -1;
   }

   for (short i = 1; i <= 4; i++) { // read 4 lines (N,E,S,W)
      if (fgets(buffer, 4096, input) == NULL && !feof(input)) { // get line from input to buffer
         fprintf(stderr, "[ERROR] Unable to read input file!\n");
         return -1;
      }
      if (i <= 3 && feof(input)) { // if file suddenly ends
         fprintf(stderr, "[ERROR] File only contain %d lines, 4 lines expected.\n", i);
         return -1;
      }
      /*
      N: 00 1E 00 00 1S 1E 00 00  buffer[4], buffer[7], buffer[10], ...
      0123456789012345678901234567
      */
      size_t j = 0;
      while (true) {
         if (buffer[j] == 0) break;
         if (j >= 4 && j%3 == 1) switch (buffer[j]) {
            case '0': carFlow[i-1].push_back(EMPTY); break;
            case 'N': carFlow[i-1].push_back(DIR_N); break;
            case 'E': carFlow[i-1].push_back(DIR_E); break;
            case 'S': carFlow[i-1].push_back(DIR_S); break;
            case 'W': carFlow[i-1].push_back(DIR_W); break;
            default: // not belong to any of the previous
               fprintf(stderr, "[ERROR] Unexpected char occured! (%c)\n", buffer[j]);
               return -1;
         }
         ++j;
      }
   }
   fclose(input);
   // input file is read successfully

   // set heuristic bound
   heu_b = (long long unsigned)(pow((double)carFlow[0].size(), 1.2) * 0.15); // exponential model

   // remove tail EMPTYs from carFlow[]
   for (size_t i = 0; i < 4; i++)
      while (carFlow[i][carFlow[i].size()-1] == EMPTY) carFlow[i].pop_back();

   // notice: root->_go=={0}, root->_late==0
   treeNode* root = new treeNode;
   try {explore(root);} catch (exception&) {return -1;}

   // parse tree (with only one branch) to carArrange[]
   treeNode* nodePtr = root;
   while (nodePtr->_child.size()) {
      nodePtr = nodePtr->_child[0];
      if (nodePtr->_go[0]==false && nodePtr->_go[1]==false && nodePtr->_go[2]==false && nodePtr->_go[3]==false) // nogo
         for (size_t i=0; i<4; i++) {
            ++lanePtr[i];
            carArrange[i].push_back(EMPTY);
         }
      else for (size_t i=0; i<4; i++) {
         if (nodePtr->_go[i]) carArrange[i].push_back(carFlow[i][lanePtr[i]++]);
         else {
            carArrange[i].push_back(EMPTY);
            while (lanePtr[i] < carFlow[i].size() && lanePtr[i] <= nodePtr->_depth-1 && carFlow[i][lanePtr[i]] == EMPTY)
               ++lanePtr[i];
         }
      }
   }

   // padding 00 at tail of carArrange[]
   size_t max = 0;
   for (size_t i=0; i<4; i++)
      max = (max < carArrange[i].size()) ? carArrange[i].size() : max;
   for (size_t i=0; i<4; i++)
      while (carArrange[i].size() < max) carArrange[i].push_back(EMPTY);

   printOut(output, carArrange); // to file
   printf("total waiting time: %d\n", root->_late);
   printf("total rounds explored: %lld\n", node_cnt);
   fclose(output);
   return 0;
}

// do conflict table
// dir[4] == { (dir of N), (dir of E), (dir of S), (dir of W) }
void conflict (Direction* dir, vector<pair<pair<unsigned,unsigned>,pair<unsigned,unsigned> > >& available) {
   available.clear();
   bool marginal[4] = {0};

   if (dir[0] == 3 && dir[1] == 0 && dir[2] == 1 && dir[3] == 2) {
      available.push_back(make_pair(make_pair(0, 1), make_pair(2, 3)));
      return;
   }

   for (size_t i = 0; i < 4; i++) for (size_t j = i+1; j < 4; j++) for (size_t k = j+1; k < 4; k++) {
   	  if (dir[i] > 3 || dir[j] > 3 || dir[k] > 3) continue;
      switch (dir[i]-i) {
         case 0: fprintf(stderr, "[ERROR] There is an U turn in the input.\n"); throw exception();
         case 1: case -3: // L turn
            continue;
         case 2: case -2: // Straight
            if (j == i-1 || j == i+3 || k == i-1 || k == i+3 || (j-k != 1 && k-j != 1)) continue;
            if ((dir[j]-j != -1 && dir[j]-j != 3) || (dir[k]-k != -1 && dir[k]-k != 3)) continue;
            break;
         case 3: case -1: // R turn
            if (dir[j]-j == 1 || dir[j]-j == -3 || dir[k]-k == 1 || dir[k]-k == -3) continue;
            if (dir[j]-j == 2 || dir[j]-j == -2) {
               if (dir[k]-k == 2 || dir[k]-k == -2) continue;
		       if (i == j-1 || i == j+3 || k == j-1 || k == j+3) continue;
            }
            if (dir[k]-k == 2 || dir[k]-k == -2)
			   if (i == k-1 || i == k+3 || j == k-1 || j == k+3) continue;
            break;
      }
      available.push_back(make_pair(make_pair(i, j), make_pair(k, 4)));
      marginal[k] = marginal[j] = marginal[i] = true;
   }

   // available.push_back(make_pair(make_pair(0, 3), make_pair(4, 4))); // lane N and W don't conflict
   for (size_t i = 0; i < 4; i++) for (size_t j = i+1; j < 4; j++) {
      if (dir[i] > 3 || dir[j] > 3) continue;
      // lookup table should cover 20/144 instances; use condition
      switch (dir[i]-i) {
         case 0: fprintf(stderr, "[ERROR] There is an U turn in the input.\n"); throw exception();
         case 1: case -3: // L turn
            if (dir[j] != i || dir[i] != j) continue; // for eats continue, switch doesn't
            break;
         case 2: case -2: // Straight
            if ((dir[j] == i) + (dir[i] == j) + (dir[j]-j == 3 || dir[j]-j == -1) != 2) continue;
            break;
         case 3: case -1: // R turn
            if (dir[j] == dir[i] || dir[j] != i && (dir[j]-j == 1 || dir[j]-j == -3)) continue;
            break;
      }
      available.push_back(make_pair(make_pair(i, j), make_pair(4, 4)));
      marginal[j] = marginal[i] = true;
   }

   // available.push_back(make_pair(make_pair(1, 4), make_pair(4, 4))); // lane E confilcts to all the others
   for (size_t i = 0; i < 4; i++) if (!marginal[i] && dir[i] < EMPTY) available.push_back(make_pair(make_pair(i, 4), make_pair(4, 4)));
}

void explore (treeNode* node) { // do main recursion
   ++node_cnt;
   vector<pair<pair<unsigned,unsigned>,pair<unsigned,unsigned> > > avails;
   Direction dir[4];

   // get pending cars, UNDEF if this lane is done
   for (size_t i = 0; i < 4; i++)
      dir[i] = (lanePtr[i] < carFlow[i].size()) ? carFlow[i][lanePtr[i]] : UNDEF;

   conflict(dir, avails);
   if (!avails.size()) {
      // if all cars are done
      if (UNDEF == dir[0] && UNDEF == dir[1] && UNDEF == dir[2] && UNDEF == dir[3]) return;

      // else, there's just no car temporarily
      bool nogo[4] = {false};

      // non-UNDEF lanes go forward
      for (size_t i = 0; i < 4; i++) if (dir[i] != UNDEF) ++lanePtr[i];

      // calculate latency for all 4 lanes
      unsigned late = 0;
      for (size_t j = 0; j < 4; j++) {
         if (carFlow[j].size())
            for (int k = lanePtr[j], n = (node->_depth < carFlow[j].size()) ? node->_depth + 1 : carFlow[j].size(); k < n; k++)
                if (carFlow[j][k] < EMPTY) ++late;
      }

      treeNode* child = new treeNode(node->_depth+1);
      child->setGo(nogo);
      child->setLate(late);
      node->appendChild(child);
      explore(child);

      // drawback lanePtr[]
      for (size_t i = 0; i < 4; i++) if (dir[i] != UNDEF) --lanePtr[i];
   }
   else {
   	  if (avails.size() > 1) { // cut off some branches which awaiting cars have smaller latency, heuristic method
   	  	 // calculate latency for 4 lanes respectively
         unsigned late[4] = {0};
         for (size_t i = 0; i < 4; i++) {
            if (carFlow[i].size())
               for (int k = lanePtr[i], n = (node->_depth < carFlow[i].size()) ? node->_depth : carFlow[i].size(); k < n; k++)
                   if (carFlow[i][k] < EMPTY) ++late[i];
         }

         // calculate total latency for each branch
         vector<pair<unsigned,size_t> > latesum;
   	     for (size_t i = 0; i < avails.size(); i++) {
   	        latesum.push_back(make_pair(0,i));
   	        latesum[i].first += late[avails[i].first.first];
   	        if (avails[i].first.second < 4) latesum[i].first += late[avails[i].first.second];
   	        if (avails[i].second.first < 4) latesum[i].first += late[avails[i].second.first];
   	        if (avails[i].second.second < 4) latesum[i].first += late[avails[i].second.second];
   	     }

   	     /* cut off branches */
   	     // keep 2 branches with highest latency
   	     sort(latesum.rbegin(),latesum.rend());
   	     if (avails.size() >= 3 && latesum[0].first > latesum[latesum.size()-1].first) {
   	     	++latesum[0].first; // assert them to be nonzero
   	     	++latesum[1].first;
   	     	for (size_t i = 2; i < latesum.size(); i++) latesum[i].first = 0;
   	     	sort(latesum.rbegin(),latesum.rend());
   	     	for (size_t i = 2; i < latesum.size(); i++) avails.erase(avails.begin()+i); // erase from end to begin
   	     	--latesum[0].first;
   	     	--latesum[1].first;
   	     }

   	     // if (1st) > (1.00X * 2nd) erase 2nd
   	     if (avails.size() == 2 && latesum[0].first*heu_b > latesum[1].first*(heu_b+1))
		    (latesum[0].second < latesum[1].second) ? avails.erase(avails.begin()) : avails.erase(avails.begin()+1);

		 // rand() % 1.004^depth*5-4 != 0
		 // if (node_cnt % (long long unsigned)(pow(1.004,node->_depth)*5.0-4.0))
		 if (node_cnt % (long long unsigned)(pow(1.0001,node->_depth)*240.0-239.0))
		    while (avails.size() > 1) avails.pop_back();
   	  }
      for (size_t i = 0; i < avails.size(); i++) {
         // parse vector of pair to bool array
         bool go[4] = {false};
         go[avails[i].first.first] = true;
         if (avails[i].first.second < 4) go[avails[i].first.second] = true;
         if (avails[i].second.first < 4) go[avails[i].second.first] = true;
         if (avails[i].second.second < 4) go[avails[i].second.second] = true;
   
         // chosen lanes go forward
         unsigned empty_go[4] = {0};
         for (size_t j = 0; j < 4; j++) {
            if (go[j]) ++lanePtr[j];
            else if (dir[j] == EMPTY) {
               while (lanePtr[j] < carFlow[j].size() && lanePtr[j] <= node->_depth && carFlow[j][lanePtr[j]] == EMPTY) {
                  ++lanePtr[j];
                  ++empty_go[j]; 
               }
            }
         }
   
         // calculate latency for all 4 lanes
         unsigned late = 0;
         for (size_t j = 0; j < 4; j++) {
            if (carFlow[j].size())
               for (int k = lanePtr[j], n = (node->_depth < carFlow[j].size()) ? node->_depth + 1 : carFlow[j].size(); k < n; k++)
                   if (carFlow[j][k] < EMPTY) ++late;
         }
   
         treeNode* child = new treeNode(node->_depth+1);
         child->setGo(go);
         child->setLate(late);
         node->appendChild(child);
         explore(child);
   
         // drawback lanePtr[]
         for (size_t j = 0; j < 4; j++) {
            if (go[j]) --lanePtr[j];
            else lanePtr[j] -= empty_go[j];
         }
      }
   } 

   // delete fat children, add child's late to self
   size_t minode = 0;
   for (size_t i = 0; i < node->_child.size(); i++)
      if (node->_child[i]->_late < node->_child[minode]->_late) minode = i;
   treeNode* minPtr = node->_child[minode];
   node->_child.erase(node->_child.begin() + minode);
   for (size_t i = 0; i < node->_child.size(); i++) delete node->_child[i];
   node->_child.clear();
   node->setLate(node->_late + minPtr->_late);
   node->appendChild(minPtr);
}

void printOut (FILE* stream, vector<Direction>* car) {
   for (size_t i=0; i<4; i++) {
      switch (i) {
         case 0: fprintf(stream, "N:"); break;
         case 1: fprintf(stream, "E:"); break;
         case 2: fprintf(stream, "S:"); break;
         case 3: fprintf(stream, "W:");
      }
      for (size_t j=0; j<car[i].size(); j++) switch (car[i][j]) {
         case DIR_N: fprintf(stream, " 1N"); break;
         case DIR_E: fprintf(stream, " 1E"); break;
         case DIR_W: fprintf(stream, " 1W"); break;
         case DIR_S: fprintf(stream, " 1S"); break;
         case EMPTY: fprintf(stream, " 00");
      }
      fprintf(stream, "\n");
   }
}
