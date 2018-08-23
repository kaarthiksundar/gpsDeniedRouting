#ifndef PLOT_H_
#define PLOT_H_

#include "utilities.h"
#include <lemon/graph_to_eps.h>


void plot(const Instance & instance, const Result & result) {

    // create a color palette
    lemon::Palette palette;

    // create a ListGraph
    lemon::ListGraph g;
	typedef lemon::ListGraph::Node Node;
	typedef lemon::ListGraph::NodeIt NodeIt;
	typedef lemon::ListGraph::Edge Edge;
	typedef lemon::dim2::Point<double> Point;

    std::vector<Node> nodes;
    nodes.resize(instance.getNumTargets() + instance.getNumLandmarks());
    for (int i=0; i<nodes.size(); ++i)
        nodes[i] = g.addNode();

    lemon::ListGraph::NodeMap<Point> coords(g);
	lemon::ListGraph::NodeMap<double> sizes(g);
	lemon::ListGraph::NodeMap<int> colors(g);
	lemon::ListGraph::NodeMap<int> shapes(g);
	lemon::ListGraph::EdgeMap<int> ecolors(g);
	lemon::ListGraph::EdgeMap<int> widths(g);

    for (int i=0; i<instance.getNumTargets(); ++i) {
        std::tuple<double, double> coordinate = instance.getTargetCoord(i);
        coords[nodes[i]] = Point(std::get<0>(coordinate), std::get<1>(coordinate));
        shapes[nodes[i]] = 0;
        sizes[nodes[i]] = 1;
        colors[nodes[i]] = 1;
    }

    for (int i=0; i<instance.getNumLandmarks(); ++i) {
        std::tuple<double, double> coordinate = instance.getLMCoord(i);
        coords[nodes[i + instance.getNumTargets()]] = Point(std::get<0>(coordinate), std::get<1>(coordinate));
        shapes[nodes[i + instance.getNumTargets()]] = 2;
        sizes[nodes[i + instance.getNumTargets()]] = 1;
        colors[nodes[i + instance.getNumTargets()]] = 2;

    }

    auto path = result.getPath();
    for (int i=0; i<path.size()-1; ++i) {
        int s = path[i]; 
        int t = path[i+1];
        Node ns = nodes[s];
        Node nt = nodes[t];
        Edge temp = g.addEdge(ns, nt); 
        widths[temp] = 1; ecolors[temp] = 3;
    }

    graphToEps(g,"solutionPlot.eps").
			coords(coords).
			title("Feasible Solution Illustration").
			nodeScale(.01).nodeSizes(sizes).
			nodeShapes(shapes).//nodeColors(composeMap(palette, colors)).
			edgeWidthScale(.005).edgeWidths(widths).
            //edgeColors(composeMap(palette, ecolors)).
			enableParallel().parArcDist(1).
			run();

    return;



}

#endif