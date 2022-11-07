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

#define WIDTH 800
#define HEIGHT 800

float halfW = WIDTH/2;
float halfH = HEIGHT/2;

glm::vec3 cameraPosition(0.0,0.0,4.0);

glm::vec3 rightO(1.0f,0.0f,0.0f);
glm::vec3 upO(0.0f,1.0f,0.0f);
glm::vec3 forwardO(0.0f,0.0f,1.0f);
glm::mat3 cameraOrientation(rightO,upO,forwardO);


float focalDistance = HEIGHT;
float xAngle = 0;
float yAngle = 0;

float orbitState = 1.55;
uint32_t drawState = 0;


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
std::unordered_map<std::string,Colour> materialMap;
std::vector<glm::vec3> allVertex;


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
    while(getline(MyReadFile,myText)){

		if(myText[0] == 'u'){
			std::vector<std::string> split = splitBySpace(myText);
			currentColVal = materialMap.at(split[1]);
		}
		
        if(myText[0] == 'v'){
			
            std::vector<std::string> listOfCoords = splitBySpace(myText);
			// std::cout << listOfCoords[2] << std::endl;
            //need to start at index 1 since 0 is 'v'
            glm::vec3 vertex(std::stof(listOfCoords[1]), std::stof(listOfCoords[2]),std::stof(listOfCoords[3]));
            allVertex.push_back(vertex);
        }
        if(myText[0] == 'f'){
            std::vector<std::string> listOfVertex = splitBySpace(myText);
            for(int i=1; i<4;i++){
                listOfVertex[i] = listOfVertex[i].substr(0,listOfVertex[i].size()-1);
            }
            glm::vec3 v0 = scale * allVertex[std::stoi(listOfVertex[1])-1];
            glm::vec3 v1 = scale * allVertex[std::stoi(listOfVertex[2])-1];
            glm::vec3 v2 = scale * allVertex[std::stoi(listOfVertex[3])-1];

            ModelTriangle triangle(v0,v1,v2,currentColVal);

            modelTriangles.push_back(triangle);
        }
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
	cameraOrientation[2] =  glm::normalize(pointTo);
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


void orbit(DrawingWindow &window){
	glm::vec3 origin(0.0,0.0,0.0);
	cameraPosition[0] = 4 * cos(orbitState);
	cameraPosition[2] = 4 * sin(orbitState);
	pointAt(cameraPosition);
	// filledFrame(cameraPosition,focalDistance,window);
	wireFrame(cameraPosition,focalDistance, window);
	orbitState+= 0.02;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) cameraPosition[0] -= 0.01;
		else if (event.key.keysym.sym == SDLK_RIGHT) cameraPosition[0] += 0.01;
		else if (event.key.keysym.sym == SDLK_UP) cameraPosition[1] += 0.01;
		else if (event.key.keysym.sym == SDLK_DOWN) cameraPosition[1] -=0.01;
		else if (event.key.keysym.sym == SDLK_w) cameraPosition[2] -=0.01;
		else if (event.key.keysym.sym == SDLK_s) cameraPosition[2] +=0.01;
		else if (event.key.keysym.sym == SDLK_o) drawState = 1;
		else if (event.key.keysym.sym == SDLK_p) drawState = 0;
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
	if (drawState == 1) orbit(window);
	
	
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


int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	
	
	setBufferToZero();
	ObjParserMaterial("cornell-box.mtl");
	ObjParserTriangle("cornell-box.obj", 0.35);
	// wireFrame(cameraPosition,focalDistance,window);
	// filledFrame(cameraPosition,focalDistance,window);
	// consoleTriangles();
	
	
	
	while (true) {
		window.clearPixels();
		setBufferToZero();
		
		
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);

		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
