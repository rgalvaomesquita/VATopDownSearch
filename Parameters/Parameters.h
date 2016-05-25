#include <iostream>

#define MAX_DIST 2
#define NUM_MIN_MATCHES 4
#define NBITS 72
#define KNN_RATIO 0.6
#define VERBOSE false
#define TRAIN 0
#define GENERATE_KP_DES false
#define FIRST_OBJECT 1
#define LAST_OBJECT 8

extern const std::string DIR_IMGS_TEST;
extern const std::string DIR_OBJECTS;
extern const std::string ROOT_DIR;
extern const std::string DIR_DES_OBJECTS;
extern const std::string DIR_TXT_RESULTS;
extern const std::string DIR_KP_OBJECTS;
extern const std::string DIR_FILTERS;
extern const std::string DIR_IMG_MATCHES;

extern std::string objs[8];
extern float percKpSal[8];
extern const bool GT[8][51];
