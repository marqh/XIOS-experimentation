#ifndef  __PARALLEL_TREE_HPP__
#define  __PARALLEL_TREE_HPP__

#include "tree.hpp" // for local tree and routing tree
//#include "sample_tree.hpp"
#include "mpi_cascade.hpp"
#include "mpi.hpp"

namespace sphereRemap {

class CParallelTree
{
public:
	CParallelTree(MPI_Comm comm);
	~CParallelTree();

	void build(vector<NodePtr>& node, vector<NodePtr>& node2);

	void routeNodes(vector<int>& route, vector<NodePtr>& nodes, int level = 0);
	void routeIntersections(vector<vector<int> >& route, vector<NodePtr>& nodes, int level = 0);

	int nbLocalElements;
	Elt* localElements;

	CTree localTree;

private:
	void updateCirclesForRouting(Coord rootCentre, double rootRadius, int level = 0);
	void buildSampleTreeCascade(vector<NodePtr>& sampleNodes, int level = 0);
	void buildLocalTree(const vector<NodePtr>& node, const vector<int>& route);
	void buildRouteTree();

	//CSampleTree sampleTree;
	vector<CSampleTree> treeCascade; // first for sample tree, then for routing tree
	CMPICascade cascade;
  MPI_Comm communicator ;

};

void buildSampleTree(CSampleTree& tree, const vector<NodePtr>& node, const CCascadeLevel& comm);

}
#endif
