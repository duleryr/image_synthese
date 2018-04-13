#include "joint.h"
#include <stack>

#include <QtGui/QMatrix4x4>

#define DEBUG_ROOT_MATRIX false
using namespace std;

const std::string kMotion = "MOTION";
const std::string kChannels = "CHANNELS";
const std::string kEnd = "End";
const std::string kFrame = "Frame";
const std::string kFrames = "Frames:";
const std::string kHierarchy = "HIERARCHY";
const std::string kJoint = "JOINT";
const std::string kOffset = "OFFSET";
const std::string kRoot = "ROOT";

const std::string kXpos = "Xposition";
const std::string kYpos = "Yposition";
const std::string kZpos = "Zposition";
const std::string kXrot = "Xrotation";
const std::string kYrot = "Yrotation";
const std::string kZrot = "Zrotation";

const std::string kOpenBracket = "{";
const std::string kCloseBracket = "}";

Joint* Joint::createFromFile(std::string fileName, int & nb_children, int & nb_indices, int & maxFrameNb, double & frameTime) {
	Joint* root = NULL;
    std::stack<Joint*> currentHierarchyStack;
    std::cout << "Loading from " << fileName << "..." << std::endl;

	ifstream inputfile(fileName.data());
	if(inputfile.good()) {
	    string buf;	
        int currentId = 0;
		while(buf != kMotion) {
			inputfile >> buf;
			// DONE : construire la structure de données root à partir du fichier
            if (buf == kHierarchy) {
                std::cerr << "Starting bvh parser" << std::endl;
            }
            if (buf == kRoot) {
                root = new Joint();
                root->_id = currentId;

			    inputfile >> buf;
                root->_name = buf;

                currentHierarchyStack.push(root);
            }
            if (buf == kOffset) {
			    inputfile >> buf;
                currentHierarchyStack.top()->_offX = std::stod(buf);
			    inputfile >> buf;
                currentHierarchyStack.top()->_offY = std::stod(buf);
			    inputfile >> buf;
                currentHierarchyStack.top()->_offZ = std::stod(buf);
            }
            if (buf == kJoint || buf == kEnd) {
                Joint * tmp = new Joint();
                currentId++;
                tmp->_id = currentId;
                (currentHierarchyStack.top()->_children).push_back(tmp);
                currentHierarchyStack.push(tmp);

			    inputfile >> buf;
                currentHierarchyStack.top()->_name = buf;
            }
            if (buf == kCloseBracket) {
                currentHierarchyStack.pop();
            }
            if (buf == kChannels) {
			    inputfile >> buf;
                int nb_channels = stoi(buf);
                for (int i = 0; i < nb_channels; i++) {
			        inputfile >> buf;
                    currentHierarchyStack.top()->_dofs.push_back(AnimCurve(buf));
                }
            }
		}
        nb_children = currentId + 1;
        nb_indices = (nb_children - 1) * 2;
		inputfile >> buf;
		inputfile >> buf;
        maxFrameNb = std::stoi(buf);
        std::cout << "Frames : " << maxFrameNb << std::endl;
		inputfile >> buf;
		inputfile >> buf;
		inputfile >> buf;
        frameTime = std::stod(buf);
        // Parcours en profondeur récursif de la structure pour remplir les listes d'AnimCurve
        for (int i = 0; i < maxFrameNb; i++) {
            root->fillAnimCurve(inputfile);
        }
		inputfile.close();
	} else {
		std::cerr << "Failed to load the file " << fileName.data() << std::endl;
		fflush(stdout);
	}

    std::cout << "file loaded" << std::endl;

	return root;
}

void Joint::fillAnimCurve(std::ifstream & inputfile) {
    string buf;
    for (int i = 0; i < this->_dofs.size(); i++) {
	    inputfile >> buf;
        this->_dofs[i]._values.push_back(std::stod(buf));
    }
    for (int i = 0; i < this->_children.size(); i++) {
        this->_children[i]->fillAnimCurve(inputfile);
    }
}

void Joint::computeIndicesArray(std::vector<int> & indicesVector) {
    recursiveIndicesArray(indicesVector);
}

void Joint::recursiveIndicesArray(std::vector<int> & inputArray) {
    for (int i = 0; i < this->_children.size(); i++) {
        inputArray.push_back(this->_id);
        inputArray.push_back(this->_children[i]->_id);
        this->_children[i]->recursiveIndicesArray(inputArray);
    }
}

// Done by Thomas
void Joint::printDataStructure(int frame_nb) {
    this->animate(frame_nb);
    std::cout << "Frame nb : " << frame_nb << std::endl;
    std::cout << _name << " " << _id << std::endl;
    std::cout << _offX << " " << _offY << " " << _offZ << std::endl;
    std::cout << _curRz << " " << _curRy << " " << _curRx << std::endl;
    for (auto i : _children) {
        std::cout << "   " << i->_name << " " << i->_id << std::endl;
        std::cout << "   offset :" << i->_offX << " " << i->_offY << " " << i->_offZ << std::endl;
        std::cout << "   rotation :" << i->_curRz << " " << i->_curRy << " " << i->_curRx << std::endl;
        for (auto j : i->_children) {
            std::cout << "      " << j->_name << " " << j->_id << std::endl;
            std::cout << "      offset :" << j->_offX << " " << j->_offY << " " << j->_offZ << std::endl;
            std::cout << "      rotation :" << j->_curRz << " " << j->_curRy << " " << j->_curRx << std::endl;
            for (auto k : j->_children) {
                std::cout << "          " << k->_name << " " << k->_id << std::endl;
                std::cout << "          offset :" << k->_offX << " " << k->_offY << " " << k->_offZ << std::endl;
                std::cout << "          rotation :" << k->_curRz << " " << k->_curRy << " " << k->_curRx << std::endl;
                for (auto l : k->_children) {
                    std::cout << "             " << l->_name << " " << l->_id << std::endl;
                    for (auto m : l->_children) {
                        std::cout << "                " << m->_name << " " << m->_id << std::endl;
                        for (auto n : m->_children) {
                            std::cout << "                   " << n->_name << " " << n->_id << std::endl;
                            for (auto o : n->_children) {
                                std::cout << "                       " << o->_name << " " << o->_id << std::endl;
                                for (auto p : o->_children) {
                                    std::cout << "                           " << p->_name << " " << p->_id << std::endl;
                                    for (auto q : p->_children) {
                                        std::cout << "                             " << q->_name << " " << q->_id << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << std::endl;
}

void Joint::animate(int iframe) 
{
	// Update dofs :
	_curTx = 0; _curTy = 0; _curTz = 0;
	_curRx = 0; _curRy = 0; _curRz = 0;
	for (unsigned int idof = 0 ; idof < _dofs.size() ; idof++) {
		if(!_dofs[idof].name.compare("Xposition")) _curTx = _dofs[idof]._values[iframe];
		if(!_dofs[idof].name.compare("Yposition")) _curTy = _dofs[idof]._values[iframe];
		if(!_dofs[idof].name.compare("Zposition")) _curTz = _dofs[idof]._values[iframe];
		if(!_dofs[idof].name.compare("Zrotation")) _curRz = _dofs[idof]._values[iframe];
		if(!_dofs[idof].name.compare("Yrotation")) _curRy = _dofs[idof]._values[iframe];
		if(!_dofs[idof].name.compare("Xrotation")) _curRx = _dofs[idof]._values[iframe];
	}	
	// Animate children :
	for (unsigned int ichild = 0 ; ichild < _children.size() ; ichild++) {
		_children[ichild]->animate(iframe);
	}
}

// Fonction toujours appelée sur Root
void Joint::computePointPositions(std::vector<trimesh::point> & positions, std::vector<glm::mat4> & transMatrix) {
    glm::mat4 rootMatrix = glm::mat4(1.0);

    rootMatrix = glm::translate(rootMatrix, glm::vec3(_offX, _offY, _offZ));

    rootMatrix = glm::translate(rootMatrix, glm::vec3(_curTx, _curTy, _curTz));

#if DEBUG_ROOT_MATRIX
    std::cout << "Transposed Root Matrix after translation : " << std::endl;
    std::cout << trimesh::point(rootMatrix[0]) << std::endl;
    std::cout << trimesh::point(rootMatrix[1]) << std::endl;
    std::cout << trimesh::point(rootMatrix[2]) << std::endl;
    std::cout << trimesh::point(rootMatrix[3]) << std::endl;
#endif

    glm::mat4 rZ = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRz*glm::pi<float>()/180), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rY = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRy*glm::pi<float>()/180), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rX = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRx*glm::pi<float>()/180), glm::vec3(1.0f, 0.0f, 0.0f));

    rootMatrix = rootMatrix * rZ * rY * rX;

	transMatrix[_id] = rootMatrix;

#if DEBUG_ROOT_MATRIX
    std::cout << "Transposed Root Matrix after rotation : " << std::endl;
    std::cout << trimesh::point(rootMatrix[0]) << std::endl;
    std::cout << trimesh::point(rootMatrix[1]) << std::endl;
    std::cout << trimesh::point(rootMatrix[2]) << std::endl;
    std::cout << trimesh::point(rootMatrix[3]) << std::endl;
#endif

    positions[0] = trimesh::point(rootMatrix[3]);

    for (auto& child : _children) {
        child->recursiveFillPointPositions(rootMatrix, positions, transMatrix);
    }
}

void Joint::recursiveFillPointPositions(glm::mat4 parentMatrix, std::vector<trimesh::point> & positions, std::vector<glm::mat4> & transMatrix) {
    glm::mat4 offset = glm::translate(glm::mat4(1.0), glm::vec3(_offX, _offY, _offZ));

    glm::mat4 rZ = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRz*glm::pi<float>()/180), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rY = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRy*glm::pi<float>()/180), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rX = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(_curRx*glm::pi<float>()/180), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 currentChildMatrix = parentMatrix * offset * rZ * rY * rX;

	transMatrix[_id] = currentChildMatrix;

    positions[_id] = trimesh::point(currentChildMatrix[3]);

    for (auto& child : _children) {
        child->recursiveFillPointPositions(currentChildMatrix, positions, transMatrix);
    }
}


// Fonction toujours appelée sur Root
void Joint::initPointPositions(std::vector<trimesh::point> & positions, trimesh::point decalage) {
    glm::mat4 rootMatrix = glm::mat4(1.0);
    rootMatrix = glm::translate(rootMatrix, glm::vec3(decalage[0], decalage[1], decalage[2]));
    positions[0] = trimesh::point(rootMatrix[3]);
    for (auto& child : _children) {
        child->recursiveinitPointPositions(rootMatrix, positions);
    }
}

void Joint::recursiveinitPointPositions(glm::mat4 parentMatrix, std::vector<trimesh::point> & positions) {
    glm::mat4 currentChildMatrix = glm::translate(glm::mat4(1.0), glm::vec3(_offX, _offY, _offZ));
    currentChildMatrix = parentMatrix * currentChildMatrix;
    positions[_id] = trimesh::point(currentChildMatrix[3]);

    for (auto& child : _children) {
        child->recursiveinitPointPositions(currentChildMatrix, positions);
    }
}

// Calcul des degrés de liberté
void Joint::nbDofs() {
	if (_dofs.empty()) return;

	double tol = 1e-4;

	int nbDofsR = -1;

	// TODO :
	cout << _name << " : " << nbDofsR << " degree(s) of freedom in rotation\n";

	// Propagate to children :
	for (unsigned int ichild = 0 ; ichild < _children.size() ; ichild++) {
		_children[ichild]->nbDofs();
	}
}
