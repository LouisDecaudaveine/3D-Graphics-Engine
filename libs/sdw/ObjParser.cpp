
#include "ObjParser.h"
#include <utility>



// std::vector<std::string> ObjParser::splitBySpace(std::string line){
//     std::string space_delimiter = " ";
//     std::vector<std::string> words{};

//     size_t pos = 0;
//     while ((pos = line.find(space_delimiter)) != std::string::npos) {
//         words.push_back(line.substr(0, pos));
//         line.erase(0, pos + space_delimiter.length());
//     }
    
//     return words;
// }


// ObjParser::ObjParser(std::string path){
//     Colour col(255,0,0);
//     std::string myText;
//     std::ifstream MyReadFile("src/objParser");
//     std::vector<glm::vec3> allVertex;
//     std::vector<ModelTriangle> allTriangles;

//     while(getline(MyReadFile,myText)){
//         if(myText[0] == 'v'){
//             std::vector<std::string> listOfCoords = ObjParser::splitBySpace(myText);
//             //need to start at index 1 since 0 is 'v'
//             glm::vec3 vertex(std::stof(listOfCoords[1]), std::stof(listOfCoords[2]),std::stof(listOfCoords[3]));
//             allVertex.push_back(vertex);
//         }
//         if(myText[0] == 'f'){
//             std::vector<std::string> listOfVertex = ObjParser::splitBySpace(myText);
//             for(int i=1; i<4;i++){
//                 listOfVertex[i] = listOfVertex[i].substr(0,listOfVertex[i].size()-1);
//             }
//             glm::vec3 v0 = allVertex[std::stoi(listOfVertex[1])-1];
//             glm::vec3 v1 = allVertex[std::stoi(listOfVertex[2])-1];
//             glm::vec3 v2 = allVertex[std::stoi(listOfVertex[3])-1];

//             ModelTriangle triangle(v0,v1,v2,col);

//             modelTriangles.push_back(triangle);
//         }
//     }
// }


// void ObjParser::consoleTriangles(){
//     for(ModelTriangle trig : modelTriangles){
//         std::cout << trig << std::endl;
//     }
// }    



