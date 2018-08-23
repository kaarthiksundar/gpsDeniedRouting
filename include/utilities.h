#ifndef UTILITIES_H_
#define UTILITIES_H_

/* Constant Definitions */

#define XTOL 1.0E-6      	// tolerance for the variables
#define CONS_TYPES 2	    // number of different types of constraints

/* Include files */

#include <ilcplex/ilocplex.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <utility>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <cmath>
#include <cassert>
#include <chrono>
#include <tuple>
#include <functional>
#include <unordered_map>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

ILOSTLBEGIN

class Instance {
	protected:
		std::string _name = "";
		std::string _path = "";
		int _numTargets;
		int _numLandmarks;
		std::vector<std::tuple<double, double>> _targetCoords;
		std::vector<std::tuple<double, double>> _lmCoords;
		double _threshold;
		std::vector<std::tuple<int, int>> _edges;
		std::vector<int> _numEdgeSegments;
		std::vector<double> _edgeLength;
		std::vector<double> _placementCost;
		std::unordered_map<int, std::vector<std::set<int>>> _edgeSets;
		int _lmCoverCount = 2;
		double _discretizationLength = 1;
		
		public:
			Instance() : _name(), _path(), _numTargets(), _numLandmarks(), _targetCoords(), 
				_lmCoords(), _threshold(), _edges(), _numEdgeSegments(), _edgeLength(), _placementCost(), 
				_edgeSets(), _lmCoverCount(2), _discretizationLength(1) {};

			void setName(std::string name) { _name = name; };
			void setPath(std::string path) { _path = path; };
			void setNumTargets(int numTargets) { _numTargets = numTargets; };
			void setNumLandmarks(int numLandmarks) { _numLandmarks = numLandmarks; };
			void setThreshold(double threshold) { _threshold = threshold; };
			void addEdge(std::tuple<int, int> edge) { _edges.push_back(edge); };
			void setLMCoverCount(int count) { _lmCoverCount = count; };
			void setDiscretizationLength(double length) { _discretizationLength = length; };

			void readData();
			void populateEdges();
			void populatePlacementCosts();

			std::string getName() const { return _name; };
			std::string getPath() const { return _path; };
			int getNumTargets() const { return _numTargets; };
			int getNumLandmarks() const { return _numLandmarks; };
			double getThreshold() const { return _threshold; };
			std::vector<std::tuple<int, int>> getEdges() const { return _edges; };
			std::vector<std::tuple<double, double>> getTargetCoords() const { return _targetCoords; };
			std::vector<std::tuple<double, double>> getLMCoords() const { return _lmCoords; };
			std::tuple<double, double> getTargetCoord(int i) const { return _targetCoords[i]; };
			std::tuple<double, double> getLMCoord(int i) const { return _lmCoords[i]; };
			int getNumEdges() const { return _edges.size(); };
			int getNumSegments(int i) const { return _numEdgeSegments[i]; };
			int from(int i) const { return std::get<0>(_edges[i]); };
			int to(int i) const { return std::get<1>(_edges[i]); };
			std::vector<double> getLengths() const { return _edgeLength; };
			double getEdgeLength(int i) const { return _edgeLength[i]; };
			std::vector<double> getPlacementCosts() const { return _placementCost; };
			double getLMPlacementCost(int i) const { return _placementCost[i]; };
			std::vector<std::set<int>> getEdgeCoveringSets(int i) const { return _edgeSets.at(i); };
			int getLMCoverCount() const { return _lmCoverCount; };
			double getDiscretizationLength() const { return _discretizationLength; };
			std::set<int> getDelta(int target) const;
			std::set<int> getGamma(std::set<int> &S) const;
			int getEdgeFromTargets(int u, int v) const;


			double computeDistance(std::tuple<double, double> i, std::tuple<double, double> j) {
				return std::hypot(std::get<0>(i) - std::get<0>(j), std::get<1>(i) - std::get<1>(j));
			};

			std::tuple<double, double> getInternalCoord(std::tuple<double, double> x1, std::tuple<double, double> x2, double lambda) {
				double x = (1-lambda) * std::get<0>(x1) + lambda * std::get<0>(x2);
				double y = (1-lambda) * std::get<1>(x1) + lambda * std::get<1>(x2);
				return std::make_tuple(x, y);
			};

};


class Result {
	protected:
		int _numTargets;
		int _gridSize;
		int _numLandmarks;
		double _threshold;
		double _discretizationLength;
		const std::vector<std::tuple<double, double>> _targetCoords;
		const std::vector<std::tuple<double, double>> _lmCoords;
		const std::vector<double> _lmPlacementCost;
		std::vector<int> _path;
		std::vector<int> _lmIndexes;
		double _placementCost;
		double _travelCost;
		double _totalCost;
		IloAlgorithm::Status _status;
		int _userCuts;
		double _computationTime;
		double _relOptGap;

	public:
		Result(const std::vector<std::tuple<double, double>> targetCoords, 
			const std::vector<std::tuple<double, double>> lmCoords, 
			const std::vector<double> lmPlacementCost, IloAlgorithm::Status status) : 
			_numTargets(targetCoords.size()), _gridSize(100), _numLandmarks(lmCoords.size()), 
			_threshold(), _discretizationLength(), 
			_targetCoords(targetCoords), _lmCoords(lmCoords), _lmPlacementCost(lmPlacementCost), 
			_path(), _lmIndexes(), _placementCost(0.0), _travelCost(0.0), _totalCost(0.0), 
			_status(status), _userCuts(0), _computationTime(0.0), _relOptGap(0.0) {};

		void setThreshold(double threshold) { _threshold = threshold; };
		void setDiscretizationLength(double discretizationLength) { _discretizationLength = discretizationLength; };
		void setPath(std::vector<int> path) { _path = path; };
		void setLMIndexes(std::vector<int> lmIndexes) { _lmIndexes = lmIndexes; };
		void setPlacementCost(double placementCost) { _placementCost = placementCost; };
		void setTravelCost(double travelCost) { _travelCost = travelCost; };
		void setTotalCost(double totalCost) { _totalCost = totalCost; };
		void setUserCuts(int userCuts) { _userCuts = userCuts; };
		void setComputationTime(double computationTime) { _computationTime = computationTime; };
		void setRelOptGap(double relOptGap) { _relOptGap = relOptGap; };

		int getNumTargets() const { return _numTargets; };
		int getGridSize() const { return _gridSize; };
		int getNumLandmarks() const { return _numLandmarks; };
		double getThreshold() const { return _threshold; };
		double getDiscretizationLength() const { return _discretizationLength; };
		std::vector<int> getPath() const { return _path; };
		std::vector<int> getLMIndexes() const { return _lmIndexes; };
		double getPlacementCost() const { return _placementCost; };
		double getTravelCost() const { return _travelCost; };
		double getTotalCost() const { return _totalCost; };
		IloAlgorithm::Status getStatus() const { return _status; };
		int getUserCuts() const { return _userCuts; };
		double getComputationTime() const { return _computationTime; };
		double getRelOptGap() const { return _relOptGap; };

		void writeToFile(std::ofstream &);
	
};

#endif
