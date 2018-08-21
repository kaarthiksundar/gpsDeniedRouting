#include "utilities.h"

void Instance::readData() {

	ifstream infile;
	std::string file = getPath() + getName();
	infile.open(file.c_str());
	printf(ANSI_COLOR_YELLOW 
		"[info] instance name: %s\n"
		ANSI_COLOR_RESET, file.c_str());
	
	if (!infile) {
		std::cerr << "Error: Instance file could not be opened" << std::endl;
		exit(1);
	}

	int numTargets, numLandmarks;

	infile >> numTargets >> numLandmarks;
	setNumTargets(numTargets); 
	setNumLandmarks(numLandmarks);

	printf(ANSI_COLOR_YELLOW
		"[info] number of targets: %d\n"
		ANSI_COLOR_RESET, getNumTargets());
	
	printf(ANSI_COLOR_YELLOW
		"[info] number of potential landmark locations: %d\n"
		ANSI_COLOR_RESET, getNumLandmarks());
	
	for (int i=0; i<getNumTargets(); ++i) {
		float x, y;
		infile >> x >> y;
		_targetCoords.push_back(std::make_tuple(x, y));
	}

	for (int i=0; i<getNumLandmarks(); ++i) {
		float x, y;
		infile >> x >> y;
		_lmCoords.push_back(std::make_tuple(x, y));
	}
	return;
};

void Instance::populateEdges() {

	for (int i=0; i<getNumTargets(); ++i) {
		for (int j=i+1; j<getNumTargets(); ++j) {
			double length = computeDistance(getTargetCoord(i), getTargetCoord(j));
			auto targeti = getTargetCoord(i);
			auto targetj = getTargetCoord(j);
			int numSegments = static_cast<int>(std::ceil(length/getDiscretizationLength()));
			std::vector<std::set<int>> edgeSet(numSegments);
			double delta = 1/static_cast<double>(numSegments);
			double lambda1 = 0.0, lambda2 = 0.0;
			for (int s=0; s<numSegments; ++s) {
				std::set<int> coveringLMs;
				std::tuple<double, double> c1, c2;
				lambda1 = s*delta; lambda2 = (s+1)*delta;
				c1 = getInternalCoord(targeti, targetj, lambda1);
				c2 = getInternalCoord(targeti, targetj, lambda2);
				for (int lm=0; lm<getNumLandmarks(); ++lm) {
					double fromDistance = computeDistance(c1, getLMCoord(lm));
					double toDistance = computeDistance(c2, getLMCoord(lm));
					if (fromDistance <= getThreshold() && toDistance <= getThreshold())
						coveringLMs.insert(lm);
				}
				edgeSet[s] = coveringLMs;
			}
			bool addEdge = true;
			for (auto &cover : edgeSet) { if (cover.size() < 2) { addEdge = false; continue; } }
			
			if (addEdge) {
				_edges.push_back(std::make_tuple(i, j));
				_numEdgeSegments.push_back(numSegments);
				_edgeLength.push_back(length);
				_edgeSets.insert(std::make_pair(_edges.size()-1, edgeSet));
			}
		}
	}
	return;
};

void Instance::populatePlacementCosts() {
	
	for (int i=0; i<getNumLandmarks(); ++i) {
		auto coord = getLMCoord(i);
		double x = std::get<0>(coord);
		double y = std::get<1>(coord);
		double cost = (10 + std::sin(0.3*x) + std::cos(0.3*y));
		_placementCost.push_back(cost);
	}
	return;
};

std::set<int> Instance::getDelta(int target) const {

	std::set<int> edgelist;
	for (int i=0; i<getNumEdges(); ++i) {
		int u = from(i);
		int v = to(i);
		if (u == target || v == target) 
			edgelist.insert(i); 
	}
	return edgelist;
};

std::set<int> Instance::getGamma(std::set<int> &S) const {

	std::set<int> edgelist;
	for (int i=0; i<getNumEdges(); ++i) {
		int u = from(i);
		int v = to(i);
		if (S.find(u) != S.end() && S.find(v) != S.end())
			edgelist.insert(i);
	}
	return edgelist;
};

int Instance::getEdgeFromTargets(int u, int v) const {

	for (int i=0; i<getNumEdges(); ++i) {
		if (u == from(i) && v == to(i))
			return i;
		if (v == from(i) && u == to(i))
			return i;
	}

    return std::numeric_limits<int>::infinity();

};

void Result::writeToFile(std::ofstream & out) {
	
	/* writing instance information */
	out << "number of targets: " << getNumTargets() << std::endl;
	out << "number of landmarks: " << getNumLandmarks() << std::endl;
	out << "grid size: " << getGridSize() << std::endl;
	out << "threshold or range of the vehicle: " << getThreshold() << std::endl;
	out << "edge discretization unit: " << getDiscretizationLength() << std::endl;
	out << std::endl;
	
	out << "target coordinates (format i x y)" << std::endl;
	for (int i=0; i<getNumTargets(); ++i) {
		double x = std::get<0>(_targetCoords[i]); 
		double y = std::get<1>(_targetCoords[i]);
		out << i << " " << std::setprecision(4) << x << " " 
			<< std::setprecision(4) << y << std::endl;
	}
	out << std::endl;

	out << "landmark coordinates (format i x y cost)" << std::endl;
	for (int i=0; i<getNumLandmarks(); ++i) {
		double x = std::get<0>(_lmCoords[i]);
		double y = std::get<1>(_lmCoords[i]);
		double cost = _lmPlacementCost[i];
		out << i << " " << std::setprecision(4) << x << " "
			<< std::setprecision(4) << y << " " 
			<< std::setprecision(4) << cost << std::endl;
	}
	out << std::endl;

	if (getStatus() == IloAlgorithm::Status::Infeasible) {
		out << "instance is " << getStatus() << std::endl;
		return;
	}
	else 
		out << "instance is " << getStatus() << std::endl;
	
	out << std::endl;

	out << "path length: " << getPath().size() << std::endl; 
	out << "target visit sequence" << std::endl;
	for (auto target : getPath()) 
		out << target << std::endl;

	out << std::endl;
	out << "number of landmarks placed: " << getLMIndexes().size() << std::endl;
	out << "landmarks indexes placed" << std::endl;
	for (auto lmIndex : getLMIndexes())
		out << lmIndex << std::endl; 
	
	out << std::endl;
	out << "cost information" << std::endl;
	out << "travel cost: " << std::setprecision(2) << fixed << getTravelCost() << std::endl;
	out << "placement cost: " << std::setprecision(2) << fixed << getPlacementCost() << std::endl;
	out << "total cost: " << std::setprecision(2) << fixed << getTotalCost() << std::endl;

	out << std::endl;
	out << "branch-cut-algorithm statistics" << std::endl;
	out << "number of user cuts added: " << getUserCuts() << std::endl;
	out << "computation time: " << std::setprecision(2) << getComputationTime() << " seconds" << std::endl;	
	if (getStatus() == IloAlgorithm::Status::Feasible) 
		out << "relative optimality gap: " << std::setprecision(4) << getRelOptGap() << std::endl;
	
	return;

};

