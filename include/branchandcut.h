#ifndef BRANCHANDCUT_H
#define BRANCHANDCUT_H

#pragma once

#include "utilities.h"
#include "model.h"
#include <set>
#include <lemon/list_graph.h>
#include <lemon/hao_orlin.h>
#include <lemon/nagamochi_ibaraki.h>
#include <lemon/connectivity.h>

void populateBranchAndCutModel(Model &model, const Instance &instance) {
    model.getModel().setName(instance.getName().c_str());
    IloNumVarArray x(model.getEnv());
    IloNumVarArray y(model.getEnv());

    model.getVariables().insert(std::make_pair("x", x));
    model.getVariables().insert(std::make_pair("y", y));

    IloNumArray placementCost(model.getEnv());
    IloNumArray travelDistance(model.getEnv());

    for (auto i=0; i<instance.getNumEdges(); ++i) {
        std::string varname = "x_" + std::to_string(instance.from(i)) +  "_" + std::to_string(instance.to(i));
        travelDistance.add(instance.getEdgeLength(i));
        IloNumVar var(model.getEnv(), 0, 1, ILOINT, varname.c_str());
        model.getVariables().at("x").add(var);
    }


    for (auto i=0; i<instance.getNumLandmarks(); ++i) {
        std::string varname = "y_" + std::to_string(i);
        placementCost.add(instance.getLMPlacementCost(i));
        IloNumVar var(model.getEnv(), 0, 1, ILOINT, varname.c_str());
        model.getVariables().at("y").add(var);
    }

    IloExpr objExpr(model.getEnv());
    objExpr = IloScalProd(travelDistance, model.getVariables().at("x")) + 
        IloScalProd(placementCost, model.getVariables().at("y"));
    model.getModel().add(IloMinimize(model.getEnv(), objExpr));

    /* degree constraints on the targets */
    for (int i=0; i<instance.getNumTargets(); ++i) {
        IloExpr expr(model.getEnv());
        auto edgeList = instance.getDelta(i);

        for (auto const & edgeId : edgeList) 
            expr += model.getVariables().at("x")[edgeId];
        
        std::string constrname = "degree_" + std::to_string(i);
        IloRange constrDegree(model.getEnv(), 2, expr, 2, constrname.c_str());
        model.getConstraints().push_back(constrDegree);
    }

    /* landmark cover constraints */
    for (int i=0; i<instance.getNumEdges(); ++i) {
        std::vector<std::set<int>> coveringSets = instance.getEdgeCoveringSets(i);
        int count = 0;
        for (auto const & coveringSet : coveringSets) {
            IloExpr expr(model.getEnv());
            for (auto const & lm : coveringSet) 
                expr += model.getVariables().at("y")[lm];
            
            expr -= (instance.getLMCoverCount()*model.getVariables().at("x")[i]);
            std:string constrname = "lmCover_edge_" + std::to_string(instance.from(i)) + "_"
                + std::to_string(instance.to(i)) + "_segment_" + std::to_string(count);
            count++;
            IloRange constrLMCover(model.getEnv(), 0, expr, IloInfinity, constrname.c_str());  
            model.getConstraints().push_back(constrLMCover);
        }
    }

    for (IloRange constr : model.getConstraints())
        model.getModel().add(constr);

    return;
};

std::vector<IloRange> generateLazyConstraints(Model & model, const Instance & instance,
                std::unordered_map<std::string, IloNumArray> &variableValues) {

    std::vector<IloRange> constraints;
    lemon::ListGraph supportGraph;
    for (int i=0; i<instance.getNumTargets(); ++i) supportGraph.addNode();

    IloNumArray xVals = variableValues.at("x");

    for (auto i=0; i<xVals.getSize(); ++i)
        if (xVals[i] > 1E-5) {
            int from = instance.from(i);
            int to = instance.to(i);
            supportGraph.addEdge(supportGraph.nodeFromId(from), supportGraph.nodeFromId(to));
        }

    lemon::ListGraph::NodeMap<int> componentMap(supportGraph);
    int numComponents = connectedComponents(supportGraph, componentMap);

    if (numComponents == 1) 
        return constraints;
    
    std::vector<std::set<int>> components(numComponents);
	for (lemon::ListGraph::NodeIt n(supportGraph); n!=lemon::INVALID; ++n)
		components[componentMap[n]].insert(supportGraph.id(n));

    for (auto & component : components) {
        IloExpr expr(model.getEnv());
        auto edgeIds = instance.getGamma(component);
        for (auto const & edgeId : edgeIds)
            expr += model.getVariables().at("x")[edgeId];
        IloRange constr(model.getEnv(), expr, component.size()-1);
        constraints.push_back(constr);
    }
    
    return constraints;
};

// 2-matching constraint separation is commented out
std::vector<IloRange> generateUserCuts(Model & model, const Instance & instance,
                std::unordered_map<std::string, IloNumArray> &variableValues) {
                
    std::vector<IloRange> constraints;
    lemon::ListGraph supportGraph, twoMatchSupportGraph;
    lemon::ListDigraph supportDigraph;
    std::set<int> targetSet;

    for (int i=0; i<instance.getNumTargets(); ++i) {
        supportGraph.addNode();
        twoMatchSupportGraph.addNode();
        supportDigraph.addNode();
        targetSet.insert(i);
    }

    IloNumArray xVals = variableValues.at("x");

    for (auto i=0; i<xVals.getSize(); ++i) 
        if (xVals[i] > 1E-5) {
            int from = instance.from(i);
            int to = instance.to(i);
            supportGraph.addEdge(supportGraph.nodeFromId(from), supportGraph.nodeFromId(to));
            supportDigraph.addArc(supportDigraph.nodeFromId(from), supportDigraph.nodeFromId(to));
            supportDigraph.addArc(supportDigraph.nodeFromId(to), supportDigraph.nodeFromId(from));
            /*
            if (xVals[i] < 1 - 1E-5) 
                twoMatchSupportGraph.addEdge(
                    twoMatchSupportGraph.nodeFromId(from), 
                    twoMatchSupportGraph.nodeFromId(to)
                    );
            */
        }

    
    lemon::ListGraph::EdgeMap<float> capacity(supportGraph);
	for (lemon::ListGraph::EdgeIt e(supportGraph); e!=lemon::INVALID; ++e) {
		int index = instance.getEdgeFromTargets(supportGraph.id(supportGraph.u(e)), supportGraph.id(supportGraph.v(e)));
		capacity[e] = xVals[index];
	}

    lemon::ListDigraph::ArcMap<float> capacityDigraph(supportDigraph);
    for (lemon::ListDigraph::ArcIt e(supportDigraph); e!=lemon::INVALID; ++e) {
		int index = instance.getEdgeFromTargets(
            supportDigraph.id(supportDigraph.source(e)), 
            supportDigraph.id(supportDigraph.target(e))
            );
		capacityDigraph[e] = xVals[index];
	}

    lemon::ListGraph::NodeMap<int> componentMap(supportGraph);
    // lemon::ListGraph::NodeMap<int> twoMatchComponentMap(twoMatchSupportGraph);
    int numComponents = connectedComponents(supportGraph, componentMap);
    // int twoMatchNumComponents = connectedComponents(twoMatchSupportGraph, twoMatchComponentMap);

    if (numComponents > 1) {
        std::vector<std::set<int>> components(numComponents);
	    for (lemon::ListGraph::NodeIt n(supportGraph); n!=lemon::INVALID; ++n)
		    components[componentMap[n]].insert(supportGraph.id(n));

        for (auto & component : components) {
            IloExpr expr(model.getEnv());
            auto edgeIds = instance.getGamma(component);
            for (auto const & edgeId : edgeIds)
                expr += model.getVariables().at("x")[edgeId];
            IloRange constr(model.getEnv(), expr, component.size()-1);
            constraints.push_back(constr);
        }
    }
    else {
        lemon::HaoOrlin<lemon::ListDigraph, lemon::ListDigraph::ArcMap<float> > mc(supportDigraph, capacityDigraph);
        mc.init(); mc.calculateOut();
        lemon::ListDigraph::NodeMap<bool> cutMap(supportDigraph);
		float cutValue = mc.minCutMap(cutMap);
        if (cutValue < 2 - 1E-2) {
            std::set<int> component;
            for (lemon::ListDigraph::NodeIt n(supportDigraph); n!=lemon::INVALID; ++n)
                if (cutMap[n] == true) component.insert(supportDigraph.id(n));
            IloExpr expr(model.getEnv());
            auto edgeIds = instance.getGamma(component);
            for (auto const & edgeId : edgeIds)
                expr += model.getVariables().at("x")[edgeId];
            IloRange constr(model.getEnv(), expr, component.size()-1);
            constraints.push_back(constr);
        }
    }

    /*
    if (twoMatchNumComponents > 1) {
        std::vector<std::set<int>> components(twoMatchNumComponents);
        for (lemon::ListGraph::NodeIt n(twoMatchSupportGraph); n!=lemon::INVALID; ++n)
            components[twoMatchComponentMap[n]].insert(twoMatchSupportGraph.id(n));
        std::vector<std::set<int>> handles;

        for (auto component : components) 
            if (component.size() >= 3) { handles.push_back(component); };

        for (auto handle : handles) {
            std::vector<int> teeth;
            std::set<int> toCheck;
            set_difference(targetSet.begin(), targetSet.end(), 
                handle.begin(), handle.end(),
                std::inserter(toCheck, toCheck.end()));
            std::set<int> intersection;
            for (auto handleTarget : handle) {
                for (auto outsideTarget : toCheck) {
                    int edgeId = instance.getEdgeFromTargets(handleTarget, outsideTarget);
                    if (edgeId == std::numeric_limits<int>::infinity()) continue;
                    bool checkIntersection = (intersection.find(handleTarget) == intersection.end()) &&
                        (intersection.find(outsideTarget) == intersection.end());
                    if (xVals[edgeId] > 1 - 1E-5) {
                        if (checkIntersection) {
                            teeth.push_back(edgeId);
                            intersection.insert(handleTarget); 
                            intersection.insert(outsideTarget);
                        }
                    }
                }
            }
            if (teeth.size() >= 3 && teeth.size()%2 != 0) {
                IloExpr expr(model.getEnv());
                auto edgeIds = instance.getGamma(handle);
                for (auto const & edgeId : edgeIds)
                    expr += model.getVariables().at("x")[edgeId];
                for (auto const & edgeId : teeth)
                    expr += model.getVariables().at("x")[edgeId];
                double rhs = handle.size() + (teeth.size() - 1)/2;
                IloRange constr(model.getEnv(), expr, rhs);
                constraints.push_back(constr);
            }
        }

    }
    */
    

    return constraints;

};

ILOLAZYCONSTRAINTCALLBACK2(addLazyCallback, Model&, model, const Instance&, instance) {
    IloEnv env = getEnv();
    std::unordered_map<std::string, IloNumArray> variableValues;
    variableValues.insert(std::make_pair("x", IloNumArray(env, model.getVariables().at("x").getSize())));
    variableValues.insert(std::make_pair("y", IloNumArray(env, model.getVariables().at("y").getSize())));
    getValues(variableValues.at("x"), model.getVariables().at("x"));
    getValues(variableValues.at("y"), model.getVariables().at("y"));

    std::vector<IloRange> constraints = generateLazyConstraints(model, instance, variableValues);

    for (IloRange constr : constraints)
        add(constr);

    variableValues.at("x").end();
    variableValues.at("y").end();
    constraints.clear();
};

ILOUSERCUTCALLBACK2(addUserCutCallback, Model&, model, const Instance&, instance) {
    IloEnv env = getEnv();
    std::unordered_map<std::string, IloNumArray> variableValues;
    variableValues.insert(std::make_pair("x", IloNumArray(env, model.getVariables().at("x").getSize())));
    variableValues.insert(std::make_pair("y", IloNumArray(env, model.getVariables().at("y").getSize())));
    getValues(variableValues.at("x"), model.getVariables().at("x"));
    getValues(variableValues.at("y"), model.getVariables().at("y"));

    std::vector<IloRange> constraints = generateUserCuts(model, instance, variableValues);

    for (IloRange constr : constraints)
        add(constr);

    variableValues.at("x").end();
    variableValues.at("y").end();
    constraints.clear();
};




#endif
