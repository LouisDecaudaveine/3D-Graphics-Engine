#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <Colour.h>
#include <glm/glm.hpp>
#include <ModelTriangle.h> 
#include <string_view>
#include <sstream> 	
#include <unordered_map>
#include <cmath>
#include <RayTriangleIntersection.h>
#include <thread>

#define WIDTH 400
#define HEIGHT 400

float halfW = WIDTH/2;
float halfH = HEIGHT/2;

glm::vec3 cameraPosition(0.0,0.0,4.0);
glm::vec3 origin(0.0f,0.0f,0.0f);

glm::vec3 rightO(1.0f,0.0f,0.0f);
glm::vec3 upO(0.0f,1.0f,0.0f);
glm::vec3 forwardO(0.0f,0.0f,1.0f);
glm::mat3 cameraOrientation(rightO,upO,forwardO);


float focalDistance = HEIGHT;
float xAngle = 0;
float yAngle = 0;

//this might come back to bite if the object gets moved
glm::vec3 lightCoord(0.6,0.3,2.0);
// glm::vec3 lightCoord(-2.5,0.7,0.4);
float orbitState = 1.55;
uint32_t drawState = 0;




//for debugging stepping through each triangle is the index of triangle being rendered
int currentTrig = 0;


std::vector<float> interpolateSingleFloats(float from, float to, size_t numberOfValues) {
	std::vector<float> result;
	float inc = (to-from)/(numberOfValues-1);
	for(size_t i=0; i<numberOfValues; i++){
		result.push_back(from+i*inc);
	}
	return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, size_t numberOfValues){
	std::vector<glm::vec3> result;
	glm::vec3 incs((to[0]-from[0])/(numberOfValues-1),(to[1]-from[1])/(numberOfValues-1),(to[2]-from[2])/(numberOfValues-1));
	for(size_t i=0; i<numberOfValues; i++){
		result.push_back(glm::vec3((from[0]+i*incs[0]),(from[1]+i*incs[1]),(from[2]+i*incs[2])));
	}
	return result;
}

void blackAndWhiteGradient(DrawingWindow &window) {
	window.clearPixels();
	size_t detail = 85;
	std::vector<float> gradient = interpolateSingleFloats(0,255,detail);
	std::vector<float> changePos = interpolateSingleFloats(0,WIDTH,detail);
	size_t pos = 0;
	for (size_t x = 0; x < window.width; x++) {
		
		if(x >= changePos[pos]) pos++;
		float col = 255 - gradient[pos];
		for (size_t y = 0; y < window.height; y++) {
			
			float red = col;
			float green = col;
			float blue = col;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			// std::cout << x << " " << y << " " << col << std::endl;
			window.setPixelColour(x, y, colour);
		}
	}
}


void colGrad(DrawingWindow &window){
	window.clearPixels();
	glm::vec3 topLeft(255, 0, 0);        // red 
	glm::vec3 topRight(0, 0, 255);       // blue 
	glm::vec3 bottomRight(0, 255, 0);    // green 
	glm::vec3 bottomLeft(255, 255, 0);   // yellow
	std::vector<glm::vec3> gradTop = interpolateThreeElementValues(topLeft,topRight,WIDTH);
	std::vector<glm::vec3> gradBottom = interpolateThreeElementValues(bottomLeft,bottomRight,WIDTH);
	std::vector<float> topToBottom = interpolateSingleFloats(0,1,HEIGHT);
	for(size_t x = 0; x<window.width; x++){
		for(size_t y=0; y<window.height; y++){
			float red = gradTop[x][0]*(1-topToBottom[y])+gradBottom[x][0]*topToBottom[y];
			float green = gradTop[x][1]*(1-topToBottom[y])+gradBottom[x][1]*topToBottom[y];
			float blue = gradTop[x][2]*(1-topToBottom[y])+gradBottom[x][2]*topToBottom[y];
			uint32_t col = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x,y,col);
		}
	}
}

void line(CanvasPoint &from, CanvasPoint &to, Colour &col, DrawingWindow &window){
	float xDist = to.x  - from.x;
	float yDist = to.y - from.y;
	float pixelsNeeded = std::max(abs(xDist),abs(yDist));
	float xStepSize = xDist/pixelsNeeded;
	float yStepSize = yDist/pixelsNeeded;
	uint32_t colour = (255 << 24) + (int(col.red)<<16) + (int(col.green)<<8) + int(col.blue);

	for(float i = 0; i<pixelsNeeded; i++){
		float x = from.x+ xStepSize * i;
		float y = from.y+ yStepSize * i;
		window.setPixelColour(round(x),round(y),colour);
	}

}

//##############DEPTH PIXEL WORK###############
//#############################################
float depthBuffer[WIDTH][HEIGHT];

void setBufferToZero(){
	for(int i=0; i<WIDTH; i++){
		for(int j=0; j<HEIGHT; j++){
			depthBuffer[i][j] = 0.0;
		}
	}
}

void depthLine(CanvasPoint &from, CanvasPoint &to, Colour &col, DrawingWindow &window){
	float xDist = to.x  - from.x;
	float yDist = to.y - from.y;
	float pixelsNeeded = std::max(abs(xDist),abs(yDist));
	float xStepSize = xDist/pixelsNeeded;
	float yStepSize = yDist/pixelsNeeded;
	uint32_t colour = (255 << 24) + (int(col.red)<<16) + (int(col.green)<<8) + int(col.blue);
	std::vector<float> depths = interpolateSingleFloats(from.depth, to.depth, pixelsNeeded);
	for(float i = 0; i<pixelsNeeded; i++){
		float x = std::round(from.x+ xStepSize * i);
		float y = std::round(from.y+ yStepSize * i);
		if(x>0 && x < WIDTH && y>0 && y < HEIGHT){
			if(depths[i]> depthBuffer[int(x)][int(y)]){
				window.setPixelColour(x,y,colour);
				depthBuffer[int(x)][int(y)] = depths[i];
			}
			if(depths[i] == depthBuffer[int(x)][int(y)] && rand() % 10 > 5){
				window.setPixelColour(x,y,colour);
			}
		}
	}
	
}


void depthFilledTriangle(CanvasTriangle &triangle, Colour &col, DrawingWindow &window){

}

//#############################################
//#############################################


//triangle no fill
void strokedTriangle(CanvasTriangle &triangle, Colour &col, DrawingWindow &window){
	line(triangle.v0(), triangle.v1(), col, window);
	line(triangle.v1(),triangle.v2(),col,window);
	line(triangle.v2(),triangle.v0(), col, window);
}


//sorts 3 triangles by height value from lowest to biggest
std::vector<CanvasPoint> orderedTriangleY(CanvasTriangle &triangle){

	CanvasPoint highestPoint;
	CanvasPoint middlePoint;
	CanvasPoint lowestPoint;
	
// this whole section could be written more cleverly
	if(triangle.v0().y <= triangle.v1().y) {
		highestPoint = triangle.v0();
		middlePoint = triangle.v1();
	}
	else {
		highestPoint = triangle.v1();
		middlePoint = triangle.v0();
	}  

	if(triangle.v2().y <= highestPoint.y){
		lowestPoint = middlePoint;
		middlePoint = highestPoint;
		highestPoint = triangle.v2();
	}
	else if(triangle.v2().y <= middlePoint.y){
		lowestPoint = middlePoint;
		middlePoint = triangle.v2();
	}
	else lowestPoint = triangle.v2();

	std::vector<CanvasPoint> solution({highestPoint,middlePoint,lowestPoint});
	return solution;
}

//splits triangle into 2 flat triangles, [0] is top and [1] is bottom
std::vector<CanvasTriangle> splitTriangle(std::vector<CanvasPoint> &orderedPoints){
	float coef = (orderedPoints[2].y - orderedPoints[1].y)/(orderedPoints[2].y - orderedPoints[0].y);
	CanvasPoint splitPoint;
	splitPoint.y = orderedPoints[1].y;
	splitPoint.x = (orderedPoints[0].x-orderedPoints[2].x)*coef + orderedPoints[2].x;
	splitPoint.depth = (orderedPoints[0].depth-orderedPoints[2].depth)*coef + orderedPoints[2].depth;

	CanvasTriangle topTrig(orderedPoints[0], orderedPoints[1], splitPoint);
	CanvasTriangle bottomTrig(orderedPoints[1], splitPoint, orderedPoints[2]);
	std::vector<CanvasTriangle> triangles  = {topTrig, bottomTrig};
	return triangles;

}

//fills flat triangle with a colour
void fillTrig(CanvasTriangle &triangle, Colour &col, DrawingWindow &window){
	int height = triangle.v2().y - triangle.v0().y + 1; 
	int yDirection = (height > 0) ? 1 : -1;
	height = std::abs(height);
	std::vector<float> linesDepthStart = interpolateSingleFloats(triangle.v0().depth, triangle.v2().depth, height);
	// for(float de : linesDepth){
	// 	std::cout<< de  << "|";
	// }
	// std::cout<<std::endl;
	std::vector<float> startCoords = interpolateSingleFloats(triangle.v0().x, triangle.v2().x,height);
	std::vector<float> startDepths = interpolateSingleFloats(triangle.v0().depth, triangle.v2().depth, height);
	std::vector<float> endCoords;
	std::vector<float> endDepths;
	//if the triangle is flat on top
	if(triangle.v0().y == triangle.v1().y){
		endCoords = interpolateSingleFloats(triangle.v1().x, triangle.v2().x,height);
		endDepths = interpolateSingleFloats(triangle.v1().depth, triangle.v2().depth, height);
	} 
	else {
		endCoords = interpolateSingleFloats(triangle.v0().x,triangle.v1().x,height);
		endDepths = interpolateSingleFloats(triangle.v0().depth, triangle.v1().depth, height);
	}
	int xFrom;
	int xTo;
	for(int i=0;i<height;i++){
		 if(startCoords[i]>endCoords[i]){
			xFrom = std::ceil(startCoords[i]);
			xTo = std::floor(endCoords[i]);
		 }  
		else {
			xTo = std::ceil(endCoords[i]);
			xFrom = std::floor(startCoords[i]);
		} 
		CanvasPoint from(xFrom, yDirection*i+triangle.v0().y, startDepths[i]);
		CanvasPoint to(xTo, yDirection*i+triangle.v0().y, endDepths[i]);
		depthLine(from,to,col,window);
	}
	
}

//draws a line of the texture
void textureLine(CanvasPoint &from, CanvasPoint &to, CanvasPoint &fromT, CanvasPoint &toT, TextureMap &texture, DrawingWindow &window){
	uint32_t pixelsNeeded = round(std::max(abs(to.x  - from.x),abs(to.y - from.y)));
	std::vector<float> allXpos = interpolateSingleFloats(from.x,to.x,pixelsNeeded);
	std::vector<float> allYpos = interpolateSingleFloats(from.y,to.y,pixelsNeeded);
	std::vector<float> allXposT = interpolateSingleFloats(fromT.x,toT.x,pixelsNeeded);
	std::vector<float> allYposT = interpolateSingleFloats(fromT.y,toT.y,pixelsNeeded);

	for(float i = 0; i<pixelsNeeded; i++){
		window.setPixelColour(floor(allXpos[i]),floor(allYpos[i]),texture.pixels[round(allYposT[i])*texture.width+round(allXposT[i])]);
	}
}

void fillTextureTriangle(CanvasTriangle &triangle, TextureMap &texture, DrawingWindow &window){
	
	Colour col(255,255,255);
	
	int triHeight = triangle.v2().y-triangle.v0().y;
	int triDirection = (triHeight>0) ? 1 : -1;
	triHeight = std::abs(triHeight);
	std::vector<float> startCoords = interpolateSingleFloats(triangle.v0().x, triangle.v2().x,triHeight);
	std::vector<float> endCoords;

	std::vector<float> startTCoordsX = interpolateSingleFloats(triangle.v0().texturePoint.x, triangle.v2().texturePoint.x,triHeight);
	std::vector<float> startTCoordsY = interpolateSingleFloats(triangle.v0().texturePoint.y, triangle.v2().texturePoint.y,triHeight);
	std::vector<float> endTCoordsX;
	std::vector<float> endTCoordsY;

	//if the triangle is flat on top
	if(triangle.v0().y == triangle.v1().y){
		endCoords = interpolateSingleFloats(triangle.v1().x, triangle.v2().x,triHeight);
		endTCoordsX = interpolateSingleFloats(triangle.v1().texturePoint.x, triangle.v2().texturePoint.x,triHeight);
		endTCoordsY = interpolateSingleFloats(triangle.v1().texturePoint.y, triangle.v2().texturePoint.y,triHeight);
	} 
	else {
		endCoords = interpolateSingleFloats(triangle.v0().x,triangle.v1().x,triHeight);
		endTCoordsX = interpolateSingleFloats(triangle.v0().texturePoint.x, triangle.v1().texturePoint.x,triHeight);
		endTCoordsY = interpolateSingleFloats(triangle.v0().texturePoint.y, triangle.v1().texturePoint.y,triHeight);
	}

	//there needs to be a consistent way for these to represent the same thing everytime
	std::vector<float> topMid = {triangle.v1().texturePoint.x-triangle.v0().texturePoint.x, triangle.v1().texturePoint.y-triangle.v0().texturePoint.y};
	std::vector<float> topBot = {triangle.v2().texturePoint.x-triangle.v0().texturePoint.x, triangle.v2().texturePoint.y-triangle.v0().texturePoint.y};

	for(int i=0;i<triHeight;i++){
		CanvasPoint from(std::round(startCoords[i]), triDirection*i+triangle.v0().y);
		CanvasPoint to(std::round(endCoords[i]), triDirection*i+triangle.v0().y);
		CanvasPoint fromT(round(startTCoordsX[i]),round(startTCoordsY[i]));
		CanvasPoint toT(round(endTCoordsX[i]),round(endTCoordsY[i]));
		textureLine(from,to,fromT,toT,texture,window);
	}
}

void texturedTriangle(CanvasTriangle &triangle, CanvasTriangle &texturedTriangle, TextureMap &texture, DrawingWindow &window){
	Colour outlineCol(255,255,255);
	Colour col(255,255,0);
	
	
	triangle.v0().texturePoint = TexturePoint(texturedTriangle.v0().x, texturedTriangle.v0().y);
	triangle.v1().texturePoint = TexturePoint(texturedTriangle.v1().x, texturedTriangle.v1().y);
	triangle.v2().texturePoint = TexturePoint(texturedTriangle.v2().x, texturedTriangle.v2().y);

	
	std::vector<CanvasPoint> orderedPoints = orderedTriangleY(triangle);
	
	if(orderedPoints[0].y != orderedPoints[1].y && orderedPoints[0].y != orderedPoints[2].y && orderedPoints[1].y != orderedPoints[2].y){
		std::vector<CanvasTriangle> triangles = splitTriangle(orderedPoints);
		
		//calculating the texture point of the split point
		float textureSplitRatio = (triangles[0].v2().y - triangle.v0().y)/(orderedPoints[2].y - orderedPoints[0].y);
		

		int splitX = round(orderedPoints[0].texturePoint.x + textureSplitRatio*(orderedPoints[2].texturePoint.x -orderedPoints[0].texturePoint.x));
		int splitY = round(orderedPoints[0].texturePoint.y + textureSplitRatio*(orderedPoints[2].texturePoint.y -orderedPoints[0].texturePoint.y));
		triangles[0].v2().texturePoint = TexturePoint(splitX,splitY);
		triangles[1].v1().texturePoint = TexturePoint(splitX,splitY);
		

		for(int i=0; i<2;i++){
			fillTextureTriangle(triangles[i], texture, window);
		}
	}
	strokedTriangle(triangle, outlineCol, window);
}

void filledTriangle(CanvasTriangle &triangle, Colour &col, DrawingWindow &window){
	// Colour outlineCol(255,255,255);

	std::vector<CanvasPoint> orderedPoints = orderedTriangleY(triangle);
	if(orderedPoints[0].y != orderedPoints[1].y && orderedPoints[0].y != orderedPoints[2].y && orderedPoints[1].y != orderedPoints[2].y){
		std::vector<CanvasTriangle> triangles  = splitTriangle(orderedPoints);
		for(int i = 0; i<2;i++){
			fillTrig(triangles[i],col, window);
		}
	}
	else{
		CanvasTriangle ordered(orderedPoints[0], orderedPoints[1], orderedPoints[2]);
		fillTrig(ordered, col, window);
	} 
	// strokedTriangle(triangle, outlineCol, window);
}







//===============OBJ PARSER======================
//===============================================

std::vector<ModelTriangle> modelTriangles;
//this is my convoluded way for finding the normals of each vertex
std::vector<std::array<int,3>> triangleNormalIndexes;
std::unordered_map<std::string,Colour> materialMap;
std::vector<glm::vec3> allVertex;
std::vector<glm::vec3> vertexNormals;


std::vector<std::string> splitBySpace(std::string line){
    char space_char = ' ';
    std::vector<std::string> words{};
	std::stringstream	iss(line);
	std::string word;
	while(getline(iss, word, space_char)){
		words.push_back(word);
	}

	return words;
}


void ObjParserMaterial(std::string path){
	std::string line;
	std::string colName = "usemtl";
	std::string currentName;
	std::ifstream myReadFile(path);
	while(getline(myReadFile, line)){
		if (line[0] == 'n'){
			std::vector<std::string> split = splitBySpace(line);
			currentName = split[1];
		}
		if (line[0] == 'K'){
			std::vector<std::string> split = splitBySpace(line);
			Colour col(std::round(255*std::stof(split[1])), std::round(255*std::stof(split[2])),std::round(255*std::stof(split[3])));
			materialMap.insert({currentName,col});
		}
	}
}
void ObjParserTriangle(std::string path, float scale){
    Colour currentColVal(255,255,255);
    std::string myText;
    std::ifstream MyReadFile(path);
    // std::vector<glm::vec3> allVertex;
    std::vector<ModelTriangle> allTriangles;
	int addVertexIndex = 0;
    while(getline(MyReadFile,myText)){

		if(myText[0] == 's'){
			std::vector<std::string> split = splitBySpace(myText);
			addVertexIndex = std::stoi(split[1]);
		}

		if(myText[0] == 'u'){
			std::vector<std::string> split = splitBySpace(myText);
			currentColVal = materialMap.at(split[1]);
		}
		
        if(myText[0] == 'v'){
			
            std::vector<std::string> listOfCoords = splitBySpace(myText);
            //need to start at index 1 since 0 is 'v'
            glm::vec3 vertex(std::stof(listOfCoords[1]), std::stof(listOfCoords[2]),std::stof(listOfCoords[3]));
            allVertex.push_back(scale * vertex);
			vertexNormals.push_back(glm::vec3(0,0,0));
        }
        if(myText[0] == 'f'){
            std::vector<std::string> listOfVertex = splitBySpace(myText);
            for(int i=1; i<4;i++){
                listOfVertex[i] = listOfVertex[i].substr(0,listOfVertex[i].size()-1);
            }
			//this is broken trying to get the indexes to match it real counter part
			std::array<int, 3> indexes {
				std::stoi(listOfVertex[1])-1,
				std::stoi(listOfVertex[2])-1,
				std::stoi(listOfVertex[3])-1
				}; 
			triangleNormalIndexes.push_back(indexes);std::vector<std::string> split = splitBySpace(myText);

            glm::vec3 v0 = allVertex[indexes[0] + addVertexIndex];
            glm::vec3 v1 = allVertex[indexes[1] + addVertexIndex];
            glm::vec3 v2 = allVertex[indexes[2] + addVertexIndex];
            ModelTriangle triangle(v0,v1,v2,currentColVal);
			//if this doesnt work then the normal is facing the opposite way
			triangle.normal = glm::normalize(glm::cross((v1-v0),(v2-v0)));

			vertexNormals[indexes[0] + addVertexIndex] += triangle.normal;
			vertexNormals[indexes[1] + addVertexIndex] += triangle.normal;
			vertexNormals[indexes[2] + addVertexIndex] += triangle.normal;
			
            modelTriangles.push_back(triangle);
        }
    }

	// //this assigns the normals for each of the vertices
	for(int i =0; i<modelTriangles.size(); i++){
		modelTriangles[i].verticesNormals[0] = glm::normalize(vertexNormals[triangleNormalIndexes[i][0]]);
		modelTriangles[i].verticesNormals[1] = glm::normalize(vertexNormals[triangleNormalIndexes[i][1]]);
		modelTriangles[i].verticesNormals[2] = glm::normalize(vertexNormals[triangleNormalIndexes[i][2]]);
	}

}


void consoleTriangles(){
    for(ModelTriangle trig : modelTriangles){
        std::cout << trig << "  " << trig.colour << std::endl;
    }
}    

//=============================================
//=============================================

//###########CAMERA INTERSECTION###############
//#############################################

glm::mat3 xRotation(){
	glm::vec3 col1(1,0,0);
	glm::vec3 col2(0,std::cos(xAngle),std::sin(xAngle));
	glm::vec3 col3(0,-(std::sin(xAngle)), std::cos(xAngle));
	glm::mat3 rotation(col1,col2,col3);

	return rotation;
}

glm::mat3 yRotation(){
	glm::vec3 col1(std::cos(yAngle),0,-(std::sin(yAngle)));
	glm::vec3 col2(0,1,0);
	glm::vec3 col3(std::sin(yAngle), 0, std::cos(yAngle));
	glm::mat3 rotation(col1,col2,col3);

	return rotation;
}

void pointAt(glm::vec3 pointTo){
	glm::vec3 vertical(0,1,0);
	//forward
	cameraOrientation[2] =  glm::normalize(cameraPosition + pointTo);
	//right
	cameraOrientation[0] = glm::cross(vertical, cameraOrientation[2]);
	//up
	cameraOrientation[1] = glm::cross(cameraOrientation[2], cameraOrientation[0]);
}

CanvasPoint getCanvasIntersectionPoint(glm::vec3 cameraPosition, glm::vec3 vertexPosition, float focalLength){
	
	glm::vec3 cameraVertexDis = (vertexPosition - cameraPosition) * cameraOrientation;

	float xIntersection = std::round(focalLength * (-1.0f*(cameraVertexDis[0])/(cameraVertexDis[2])) + (WIDTH/2.0f));
	float yIntersection = std::round(focalLength * ((cameraVertexDis[1])/(cameraVertexDis[2])) + (HEIGHT/2.0f));
	float distanceCV = std::sqrt(std::pow(cameraVertexDis[0],2) + std::pow(cameraVertexDis[1],2) + std::pow(cameraVertexDis[2],2));
	float depth = 1.0f / (distanceCV);
	CanvasPoint intersectionPoint(xIntersection,yIntersection,depth);
	return intersectionPoint;
}

void vertexRoom(glm::vec3 cameraPosition, float focalLength, DrawingWindow &window){
	Colour col(255,255,255);
	
	for(glm::vec3 vertex : allVertex){
		CanvasPoint intersected = getCanvasIntersectionPoint(cameraPosition,vertex, focalLength);
		uint32_t col = (int(255) << 24) + (int(255) << 16) + (int(255) << 8) + int(255);
		window.setPixelColour(std::round(intersected.x),std::round(intersected.y),col);
	}
}

void wireFrame(glm::vec3 cameraPosition, float focalLength, DrawingWindow &window){
	Colour col(255,255,255);
	for(ModelTriangle trig : modelTriangles){
		col = trig.colour;
		CanvasPoint v0 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[0], focalLength);
		CanvasPoint v1 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[1], focalLength);
		CanvasPoint v2 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[2], focalLength);
		CanvasTriangle canvTrig(v0,v1,v2);
		// std::cout << canvTrig << std::endl;
		strokedTriangle(canvTrig, col, window);
	}
}

void filledFrame(glm::vec3 cameraPosition, float focalLength, DrawingWindow &window){
	Colour col(255,255,255);
	for(ModelTriangle trig : modelTriangles){
		col = trig.colour;
		CanvasPoint v0 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[0], focalLength);
		CanvasPoint v1 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[1], focalLength);
		CanvasPoint v2 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[2], focalLength);
		CanvasTriangle canvTrig(v0,v1,v2);
		filledTriangle(canvTrig,col,window);
	}
}


//#############################################
//#############################################

//###############RAY-TRACING###################
//#############################################

glm::vec3 testRay(0,0.1,-1.0);
float coolVisual = 1.0;

RayTriangleIntersection  getClosestIntersection(glm::vec3 rayDirection,glm::vec3 startPoint){
	RayTriangleIntersection closest = RayTriangleIntersection();
	closest.distanceFromCamera = 100.0f;
	closest.triangleIndex = -1;
	for(uint32_t i =0; i<modelTriangles.size(); i++){
		ModelTriangle triangle = modelTriangles[i];
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		glm::vec3 SPVector = startPoint - triangle.vertices[0];
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		float u = possibleSolution[1];
		float v = possibleSolution[2];
		if(possibleSolution[0]<closest.distanceFromCamera &&  possibleSolution[0] >= 0.001 && (u >= 0.0) && (u <= 1.0) && (v >= 0.0) && (v <= 1.0) && (u + v) <= 1.0f){
			glm::vec3 pointPos = triangle.vertices[0] + u*e0 + v*e1;
			closest = RayTriangleIntersection(pointPos, possibleSolution[0], triangle, i, u, v);
		} 
	}
	return closest;
}	

glm::vec3 getRayDirection(int x, int y){
	glm::vec3 rayDirection(0.0f,0.0f,-1.0f);
	float rayX = (x- WIDTH/2)/focalDistance;
	float rayY = -1*(y- HEIGHT/2)/focalDistance;
	rayDirection[0] = rayX;
	rayDirection[1] = rayY;
	rayDirection = cameraOrientation * glm::normalize(rayDirection);
	return rayDirection;
}

float proximityLighting(float lightToPointDis){
	float res =  3 / ( 4*3.141592*std::pow(lightToPointDis,2) ) ;
	return res > 1 ? 1 : res;
}

float angleOfIncidence(glm::vec3 surfaceNormal, glm::vec3 PointToLight){
	float res = glm::dot(surfaceNormal, PointToLight);
	return res < 0 ? 0 : res;
}

glm::vec3 pointReflection(glm::vec3 surfaceNormal, glm::vec3 LightToPoint){
	glm::vec3 reflection = LightToPoint - 2.0f * surfaceNormal * glm::dot(LightToPoint, surfaceNormal);
	return reflection;
}

glm::vec3 specularColour(glm::vec3 colour, glm::vec3 pointToCamera, glm::vec3 pointReflection){
	float specularExpo = std::pow(glm::dot(pointToCamera, pointReflection),64);
	for(int i = 0; i < 3; i++){
		colour[i] += std::floor(254* specularExpo);
		if(colour[i] > 255) colour[i] = 254;
	}
	return colour;
}

uint8_t rayTracedState = 0;
// this version may still change but I want to go from point to light not the other way 
void  RayTracedRefactored(DrawingWindow&window){
	for(uint16_t y=0; y<HEIGHT; y++){
		for(uint16_t x=0; x<WIDTH; x++){
			glm::vec3 rayDirection = getRayDirection(x,y);
			RayTriangleIntersection closestIntersection = getClosestIntersection(rayDirection, cameraPosition);
			glm::vec3 colour(0,0,0);
			float proximityLight;
			float angleToLight;

			if(closestIntersection.triangleIndex != -1){
				colour[0] =  closestIntersection.intersectedTriangle.colour.red; 
				colour[1] =  closestIntersection.intersectedTriangle.colour.green; 
				colour[2] =  closestIntersection.intersectedTriangle.colour.blue;

				glm::vec3 pointToLightRay = lightCoord - closestIntersection.intersectionPoint;
				float pointToLightDistance = glm::length(pointToLightRay);
				pointToLightRay = glm::normalize(pointToLightRay);

				RayTriangleIntersection pointToLightIntersection = getClosestIntersection(pointToLightRay,closestIntersection.intersectionPoint);
				if(pointToLightIntersection.distanceFromCamera >= pointToLightDistance){
					glm::vec3 normal = closestIntersection.intersectedTriangle.normal;
					if(rayTracedState == 0){
						normal = glm::normalize(
							(closestIntersection.v * closestIntersection.intersectedTriangle.verticesNormals[2]) + 
							(closestIntersection.u * closestIntersection.intersectedTriangle.verticesNormals[1]) + 
							(closestIntersection.w * closestIntersection.intersectedTriangle.verticesNormals[0])
						); 

						proximityLight = 0.5f * proximityLighting(pointToLightDistance);
						angleToLight = 0.5f * std::pow(angleOfIncidence(normal, pointToLightRay),1);
						colour = (proximityLight + angleToLight) * colour; 
						colour = specularColour(colour, (-1.0f * rayDirection), pointReflection(normal, (-1.0f * pointToLightRay)));
					}
				}
				else colour = 0.05f * colour;
			}
			uint32_t col = (255 << 24) + (int(colour[0])<<16) + (int(colour[1])<<8) + int(colour[2]);
			window.setPixelColour(x,y,col);
		}
	}
}
//#############################################
//#############################################

//################DEBUGGING####################
//#############################################
bool printState = true;
void oneStepRender(DrawingWindow &window){
	ModelTriangle trig = modelTriangles[currentTrig];
	if(printState){
		std::cout << "triangle index: " << currentTrig << std::endl;
		std::cout << trig << std::endl;	
	}
	// really stupid way of making sure this prints once
	printState = false;
	
	Colour col = trig.colour;
	CanvasPoint v0 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[0], focalDistance);
	CanvasPoint v1 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[1], focalDistance);
	CanvasPoint v2 = getCanvasIntersectionPoint(cameraPosition,trig.vertices[2], focalDistance);
	CanvasTriangle canvTrig(v0,v1,v2);
	filledTriangle(canvTrig,col,window);
	// wireFrame(cameraPosition, focalDistance, window);
}

void next(){
	if(currentTrig < modelTriangles.size()) currentTrig++;
	else currentTrig = 0;
}

void back(){
	if(currentTrig > 0) currentTrig--;
	else currentTrig = modelTriangles.size()-1;
}


//##############################################
//##############################################

void orbit(DrawingWindow &window){
	
	cameraPosition[0] = 4 * cos(orbitState);
	cameraPosition[2] = 4 * sin(orbitState);
	pointAt(origin);
	// filledFrame(cameraPosition,focalDistance,window);
	wireFrame(cameraPosition,focalDistance, window);
	orbitState+= 0.02;

	// RayTracedRefactored(window);
}
bool rayTraced = true;
void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			lightCoord[0] -= 0.1;
			rayTraced =false;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			lightCoord[0] += 0.1;
			rayTraced =false;
		}
		else if (event.key.keysym.sym == SDLK_UP) {
			lightCoord[2] += 0.1;
			rayTraced =false;
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			lightCoord[2] -=0.1;
			rayTraced =false;
		}
		// else if (event.key.keysym.sym == SDLK_LEFT) cameraPosition[0] -= 0.01;
		// else if (event.key.keysym.sym == SDLK_RIGHT) cameraPosition[0] += 0.01;
		// else if (event.key.keysym.sym == SDLK_UP) cameraPosition[1] += 0.01;
		// else if (event.key.keysym.sym == SDLK_DOWN) cameraPosition[1] -=0.01;
		else if (event.key.keysym.sym == SDLK_w) cameraPosition[2] -=0.01;
		else if (event.key.keysym.sym == SDLK_s) cameraPosition[2] +=0.01;
		else if (event.key.keysym.sym == SDLK_r) drawState = 4;	
		else if (event.key.keysym.sym == SDLK_d) drawState = 2;	
		else if (event.key.keysym.sym == SDLK_o) drawState = 1;
		else if (event.key.keysym.sym == SDLK_p) drawState = 0;
		else if (event.key.keysym.sym == SDLK_z) drawState = 3;
		else if (event.key.keysym.sym == SDLK_n) {
			next();
			printState = true;
			oneStepRender(window);
		}
		else if (event.key.keysym.sym == SDLK_b) {
			back();
			printState = true;
			oneStepRender(window);
		}
		else if (event.key.keysym.sym == SDLK_i){
			xAngle =0.005f;
			cameraOrientation = xRotation() * cameraOrientation;
		} 
		else if (event.key.keysym.sym == SDLK_k){
			xAngle =-0.005f;
			cameraOrientation = xRotation() * cameraOrientation;
		} 
		else if (event.key.keysym.sym == SDLK_j){
			yAngle =-0.005f;
			cameraOrientation = yRotation() * cameraOrientation;
		} 
		else if (event.key.keysym.sym == SDLK_l){
			yAngle =0.005f;
			cameraOrientation = yRotation() * cameraOrientation;
		} 
		else if (event.key.keysym.sym == SDLK_u) {
			Colour col(rand()%255,rand()%255,rand()%255);
			window.clearPixels();
			CanvasPoint v0(625.778, 572.306);
			CanvasPoint v1(266.295, 485.068);
			CanvasPoint v2(176.981, 572.306);
			CanvasTriangle trig(v0,v1,v2);
			filledTriangle(trig,col,window);

			CanvasPoint v3(rand()%WIDTH, rand()%HEIGHT);
			CanvasPoint v4(rand()%WIDTH, rand()%HEIGHT);
			CanvasPoint v5(rand()%WIDTH, rand()%HEIGHT);
			CanvasTriangle trigi(v3,v4,v5);
			filledTriangle(trigi,col,window);
			
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}


void draw(DrawingWindow &window){
	window.clearPixels();
	if(drawState == 0) filledFrame(cameraPosition,focalDistance,window);
	if(drawState == 1) orbit(window);
	if(drawState == 2) oneStepRender(window);
	if(drawState == 3)wireFrame(cameraPosition,focalDistance, window);
	
	// wireFrame(cameraPosition, focalDistance, window);


	// //canvas triangle
	// CanvasPoint v0(160, 10);
	// CanvasPoint v1(300, 230);
	// CanvasPoint v2(10, 150);
	// CanvasTriangle trig(v0,v1,v2);

	// //texture triangle
	// CanvasPoint v3(195, 5);
	// CanvasPoint v4(395, 380);
	// CanvasPoint v5 (65, 330);
	// CanvasTriangle trigTexture(v3,v4,v5);

	// std::string filePath = "src/texture.ppm";
	// TextureMap textureMap(filePath);
	// texturedTriangle(trig,trigTexture,textureMap,window);
}


//###########THREADING################
//####################################

void threadedRender(uint16_t threadCount){
	std::vector<std::thread> threads;
	for(uint16_t t=0; t<threadCount; t++){
		threads.push_back( std::thread());
	}
	for(std::thread &th : threads){
		if (th.joinable()) th.join();
	}
}

//####################################
//####################################

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	
	
	setBufferToZero();
	ObjParserMaterial("cornell-box.mtl");
	ObjParserTriangle("cornell-box.obj", 0.35);
	// ObjParserTriangle("sphere.obj", 0.35);


	uint32_t colour = (255 << 24) + (int(255)<<16) + (int(0)<<8) + int(150);
	// wireFrame(cameraPosition,focalDistance,window);
	// filledFrame(cameraPosition,focalDistance,window);
	// consoleTriangles();

	while (true) {

		if(drawState <4){
			window.clearPixels();
			draw(window);
			rayTraced = false;
		}
		else if(!rayTraced && drawState == 4){
			
			RayTracedRefactored(window);
			// std::cout<< countBehind << std::endl;
			// countBehind = 0;
			coolVisual-= 0.05;
			CanvasPoint lightCenter = getCanvasIntersectionPoint(cameraPosition,lightCoord, focalDistance);
			window.setPixelColour(lightCenter.x,lightCenter.y,colour);
			window.setPixelColour(lightCenter.x + 1,lightCenter.y,colour);
			window.setPixelColour(lightCenter.x - 1,lightCenter.y,colour);
			window.setPixelColour(lightCenter.x,lightCenter.y+1,colour);
			window.setPixelColour(lightCenter.x - 1,lightCenter.y-1,colour);
			rayTraced = true;
			std::cout<< lightCoord[0] << "," << lightCoord[1] << "," << lightCoord[2] << std::endl;
		}
		
		setBufferToZero();
		
		
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
