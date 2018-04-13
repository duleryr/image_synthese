#ifndef _JOINT_H_
#define _JOINT_H_

#include "TriMesh.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <QMatrix4x4> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class AnimCurve {
public :
	AnimCurve() {};
	AnimCurve(std::string n_name) : name(n_name) {};
	~AnimCurve() {
		_values.clear();
	}
public :
	std::string name;					// name of dof
	std::vector<double> _values;		// different keyframes = animation curve
};


enum RotateOrder {roXYZ=0, roYZX, roZXY, roXZY, roYXZ, roZYX};

class Joint {
public :
	std::string _name;					// name of joint
	double _offX;						// initial offset in X
	double _offY;						// initial offset in Y
	double _offZ;						// initial offset in Z
	std::vector<AnimCurve> _dofs;		// keyframes : _animCurves[i][f] = i-th dof at frame f;
	double _curTx;						// current value of translation on X
	double _curTy;						// current value of translation on Y
	double _curTz;						// current value of translation on Z
	double _curRx;						// current value of rotation about X (deg)
	double _curRy;						// current value of rotation about Y (deg)
	double _curRz;						// current value of rotation about Z (deg)
	int _rorder;						// order of euler angles to reconstruct rotation
	std::vector<Joint*> _children;	// children of the current joint

public :
	// Constructor :
	Joint() {};
	// Destructor :
	~Joint() {
		_dofs.clear();
		_children.clear();
	}

	// Create from data :
	static Joint* create(std::string name, double offX, double offY, double offZ, Joint* parent) {
		Joint* child = new Joint();
		child->_name = name;
		child->_offX = offX;
		child->_offY = offY;
		child->_offZ = offZ;
		child->_curTx = 0;
		child->_curTy = 0;
		child->_curTz = 0;
		child->_curRx = 0;
		child->_curRy = 0;
		child->_curRz = 0;
		if(parent != NULL) {
			parent->_children.push_back(child);
		}
		return child;
	}

	// Load from file (.bvh) :	
	static Joint* createFromFile(std::string fileName, int & nb_children, int & nb_indices, int & maxFrameNb, double & frameTime);

    void printDataStructure(int frame_nb);

	void animate(int iframe=0);

	// Analysis of degrees of freedom :
	void nbDofs();
    void computeIndicesArray(std::vector<int> & indicesVector);
	void initPointPositions(std::vector<trimesh::point> & positionsArray, trimesh::point decalage);
    void computePointPositions(std::vector<trimesh::point> & positions, std::vector<glm::mat4> & transMatrix);

private :
    int _id;           // id of Joint
    void fillAnimCurve(std::ifstream & inputfile);
    void recursiveFillPointPositions(glm::mat4 parentMatrix, std::vector<trimesh::point> & positions, std::vector<glm::mat4> & transMatrix);
    void recursiveinitPointPositions(glm::mat4 parentMatrix, std::vector<trimesh::point> & positions);
    void recursiveIndicesArray(std::vector<int> & inputArray);
	void updatePositions();
};


#endif
