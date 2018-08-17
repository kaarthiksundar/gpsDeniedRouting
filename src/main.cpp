#include "parse.h"
#include "branchandcut.h"

using namespace std;


int main(int argc, char **argv) {

	CLIParser parser(argc, argv);
	parser.parse();
	Instance instance;
	
	bool writeLog = parser.getWriteLog();
	bool writeLP = parser.getWriteLP();
	bool cplexOutput = parser.getCplexOutput();

	instance.setName(parser.getInstanceName());
	instance.setPath(parser.getInstancePath());
	instance.setThreshold(parser.getThreshold());
	instance.setDiscretizationLength(parser.getDiscretizationLength());
	instance.readData();
	instance.populateEdges();
	instance.populatePlacementCosts();

	Model model;

	try { 
		populateBranchAndCutModel(model, instance);

		IloCplex cplex(model.getModel());
		cplex.extract(model.getModel());
		if (writeLP) cplex.exportModel((parser.getLPFile()).c_str());
		cplex.use(addLazyCallback(model.getEnv(), model, instance));
		cplex.use(addUserCutCallback(model.getEnv(), model, instance));

		/* set cplex parameters */
		cplex.setParam(cplex.PreInd, 0);
		// cplex.setParam(cplex.CutsFactor, 1);
		cplex.setParam(IloCplex::MIPInterval, 5);
		cplex.setParam(IloCplex::MIPDisplay, 3);
		cplex.setParam(IloCplex::VarSel, 3);
		cplex.setParam(IloCplex::TiLim, 7200);
		cplex.setParam(IloCplex::LBHeur, 1);

		ofstream outfile;
		outfile.open(parser.getLogFile());
		if (!cplexOutput) cplex.setOut(model.getEnv().getNullStream());
		if (writeLog) cplex.setOut(outfile);
		auto start = std::chrono::system_clock::now();
		cplex.solve();
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> computationTime = end - start;
		outfile.close();
		IloAlgorithm::Status status = cplex.getStatus();

		Result result(instance.getTargetCoords(), 
			instance.getLMCoords(), 
			instance.getPlacementCosts(),
			status);
		result.setThreshold(instance.getThreshold());
		result.setDiscretizationLength(instance.getDiscretizationLength());
		result.setComputationTime(computationTime.count());
		
		ofstream resultOutfile;
		resultOutfile.open(parser.getResultFile());
		
		if (result.getStatus() == IloAlgorithm::Status::Infeasible) {
			result.writeToFile(resultOutfile);
			model.clearEnv();
			return 0;
		}

		double travelCost = 0.0;
		double placementCost = 0.0;
		double totalCost = 0.0;
		
		std::vector<int> edgeIndexes;
		for (int i=0; i<model.getVariables().at("x").getSize(); ++i) {
			if (cplex.getValue(model.getVariables().at("x")[i]) > 0.9) {
				edgeIndexes.push_back(i);
				travelCost += instance.getEdgeLength(i);
			}
		}
				
        std::vector<int> path; 
		auto edges = instance.getEdges();
        path.push_back(0);
        for (int i=0; i<edgeIndexes.size(); ++i) {
            int edgeIndex = edgeIndexes[i];
			if (instance.from(edgeIndex) == 0) {
				path.push_back(instance.to(edgeIndex));
				edgeIndexes.erase(edgeIndexes.begin() + i);
				break;
			}
			if (instance.to(edgeIndex) == 0) {
				path.push_back(instance.from(edgeIndex));
				edgeIndexes.erase(edgeIndexes.begin() + i);
				break;
			}
        }

		while (path.back() != 0) {
            int j = path.back();
            for (int i=0; i<edgeIndexes.size(); ++i) {
                int edgeIndex = edgeIndexes[i];
                if (instance.from(edgeIndex) == j) {
                    path.push_back(instance.to(edgeIndex));
                    edgeIndexes.erase(edgeIndexes.begin() + i);
                    break;
                }
                if (instance.to(edgeIndex) == j) {
                    path.push_back(instance.from(edgeIndex));
                    edgeIndexes.erase(edgeIndexes.begin() + i);
                    break;
                }
            }
        }

		assert(path.size() == instance.getNumTargets() + 1);
		result.setPath(path);
		result.setTravelCost(travelCost);

		std::vector<int> lmIndexes;
		for (int i=0; i<model.getVariables().at("y").getSize(); ++i) {
			if (cplex.getValue(model.getVariables().at("y")[i]) > 0.9) {
                lmIndexes.push_back(i);
				placementCost += instance.getLMPlacementCost(i);
			}
		}

		assert(std::abs(placementCost + travelCost - cplex.getObjValue()) < 1E-5);
		result.setLMIndexes(lmIndexes);
		result.setPlacementCost(placementCost);
		result.setTotalCost(placementCost + travelCost);
		result.setUserCuts(cplex.getNcuts(IloCplex::CutType::CutUser));
		result.writeToFile(resultOutfile);
		printf(ANSI_COLOR_YELLOW 
			"[info] results written to %s\n"
			ANSI_COLOR_RESET, parser.getResultFile().c_str());
	}

	catch (IloException& ex) { cerr << "Error: " << ex << endl; }
	catch (...) { cerr << "Error" << endl; }

	model.clearEnv();
	return 0;
}
