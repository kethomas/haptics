/*
 *
 *    DH2660 Haptic Programming Spring 2017
 *    HapMap - Group 5 (Thea, Linnéa, Kirsten)
 *    Code based on Chai3D Examples 14, 21
 *
 *    Distribution license: BSD (e.g. free to use for
 *    most purposes, see end of file)
 *
 */

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled 
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight *light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a few mesh objects
cMultiMesh* object;
cMesh* object1;
cMesh* object2;
cMesh* object3;

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// add labels for buildings
cLabel* buildingLabel;
cLabel* buildingLabel2;
cLabel* buildingLabel3;
cLabel* buildingLabel4;
cLabel* buildingLabel5;
cLabel* buildingLabel6;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// display options
bool showEdges = true;
bool showTriangles = true;
bool showNormals = false;

// display level for collision tree
int collisionTreeDisplayLevel = 0;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// root resource path
string resourceRoot;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);


//==============================================================================
/*
    DEMO:   14-textures.cpp

    This example illustrates the use of haptic textures projected onto mesh
    surfaces.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 14-textures" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "HapMap", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

    // initialize GLEW library
#ifdef GLEW_VERSION
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(0.7, 0.0, 0.2),    // SET Z TO 0.2!!!! camera position (eye)
                cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
                cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 100);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(1.5);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // enable multi-pass rendering to handle transparent objects
    camera->setUseMultipassTransparency(true);

    // create a light source
    light = new cDirectionalLight(world);

    // enable light source
    light->setEnabled(true);

    // attach light to camera
    world->addChild(light);
    camera->addChild(light);

    // define the direction of the light beam
    light->setDir(-3.0,-0.5, 0.0);

    // set lighting conditions
    light->m_ambient.set(1.0f, 1.0f, 1.0f);
    light->m_diffuse.set(0.8f, 0.8f, 0.8f);
    light->m_specular.set(1.0f, 1.0f, 1.0f);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a 3D tool and add it to the world
    tool = new cToolCursor(world);
    world->addChild(tool);

    // connect the haptic device to the tool
    tool->setHapticDevice(hapticDevice);

    // if the haptic device has a gripper, enable it as a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    // set radius of tool
    double toolRadius = 0.005;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // hide the device sphere. only show proxy.
    tool->setShowContactPoints(true, false);

    // create a white cursor
    tool->m_hapticPoint->m_sphereProxy->m_material->setBlueCadet();

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(0.25);

    // oriente tool with camera
    tool->setLocalRot(camera->getLocalRot());

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();


    //--------------------------------------------------------------------------
    // CREATE OBJECTS
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    //double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // properties
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness;// / workspaceScaleFactor;


    /////////////////////////////////////////////////////////////////////////
    // OBJECT 0: KTH Map - Thea, Linnéa, Kirsten
    ////////////////////////////////////////////////////////////////////////

    // create a multimesh
    object = new cMultiMesh();

    // add object to world
    world->addChild(object);

    // set the position of the object
    object->setLocalPos(0, 0, 0);

    // set graphic properties
    bool fileload;
    fileload = object->loadFromFile("image_objects/kth_campus.obj");
    if (!fileload)
    {
        #if defined(_MSVC)
        fileload = object->loadFromFile("image_objects/kth_campus_with_arches.obj");
        #endif
    }
    if (!fileload)
    {
        cout << "Error -  image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // set material of object
    cMaterial m;
    m.setWhite();
    object->setMaterial(m);
    // object->setTransparencyLevel(0.5);

    // disable culling so that faces are rendered on both sides
    object->setUseCulling(false);

    // compute a boundary box
    object->computeBoundaryBox(true);

    // show/hide boundary box
    object->setShowBoundaryBox(false);

    // create collision detector
    object->createAABBCollisionDetector(toolRadius);

    // center object in scene
    object->setLocalPos(-1.0 * object->getBoundaryCenter());
    std::cout <<"Position: "<< object->getLocalPos() << std::endl;
    object->setLocalPos(0, 0, 0.05);


    // compute all edges of object for which adjacent triangles have more than 40 degree angle
    object->computeAllEdges(0);

    // set line width of edges and color
    cColorf colorEdges;
    colorEdges.setBlack();
    object->setEdgeProperties(1, colorEdges);

    // set normal properties for display
    cColorf colorNormals;
    colorNormals.setOrangeTomato();
    object->setNormalsProperties(0.01, colorNormals);

    // set haptic properties
    object->setStiffness(0.3*maxStiffness);
    // can add friction and texture

    // display options
    object->setShowTriangles(showTriangles);
    object->setShowEdges(showEdges);
    object->setShowNormals(showNormals);

    /////////////////////////////////////////////////////////////////////////
    // OBJECT 1: Plane - Thea, Linnéa, Kirsten
    ////////////////////////////////////////////////////////////////////////

    // create a multimesh
    object1 = new cMesh();

    // create plane
    cCreatePlane(object1, 1.0, 1.0);

    // create collision detector
    object1->createAABBCollisionDetector(toolRadius);

    // add object to world
    world->addChild(object1);

    // set the position of the object
    object1->setLocalPos(0, 0, 0.05);

    // set graphic properties
    // bool fileload;
    object1->m_texture = cTexture2d::create();
    fileload = object1->loadFromFile("image_objects/blackstone.jpg");
    if (!fileload)
    {
        #if defined(_MSVC)
        fileload = object1->loadFromFile("image_objects/blackstone.jpg");
        #endif
    }
    if (!fileload)
    {
        cout << "Error -  Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // enable texture mapping
    object1->setUseTexture(true);
    object1->m_material->setWhite();

    // create normal map from texture data
    cNormalMapPtr normalMap1 = cNormalMap::create();
    normalMap1->createMap(object1->m_texture);
    object1->m_normalMap = normalMap1;
    normalMap1->setTextureUnit(GL_TEXTURE0_ARB);

    // set haptic properties
    object1->m_material->setStiffness(0.5 * maxStiffness);
    object1->m_material->setStaticFriction(0.3);
    object1->m_material->setDynamicFriction(0.3);
    object1->m_material->setTextureLevel(0.2);
    object1->m_material->setHapticTriangleSides(true, false);

//     // set material of object
//     cMaterial p;
//     p.setGray();
//     object1->setMaterial(p);

//     // disable culling so that faces are rendered on both sides
//     object1->setUseCulling(false);

//     // compute a boundary box
//     object1->computeBoundaryBox(true);

//     // show/hide boundary box
//     object1->setShowBoundaryBox(false);

    

//     // center object in scene
//     //object1->setLocalPos(-1.0 * object->getBoundaryCenter());

//     // compute all edges of object for which adjacent triangles have more than 40 degree angle
//     object1->computeAllEdges(0);

//     // set line width of edges and color
// //    cColorf colorEdges;
// //    colorEdges.setBlack();
// //    object->setEdgeProperties(1, colorEdges);

//     // set normal properties for display
// //    cColorf colorNormals;
// //    colorNormals.setOrangeTomato();
// //    object->setNormalsProperties(0.01, colorNormals);

//     // set haptic properties
//     object1->setStiffness(0.3 * maxStiffness);
//     object1->setFriction(0.5, 0.5);
//     // can add friction and texture

//     // display options
//     object1->setShowTriangles(showTriangles);
//     object1->setShowEdges(false);
//     object1->setShowNormals(false);


    /////////////////////////////////////////////////////////////////////////
    // OBJECT 2: Grass Texture - Thea, Linnéa, Kirsten
    ////////////////////////////////////////////////////////////////////////

    // create a mesh
    object2 = new cMesh();

    // create plane
    cCreatePlane(object2, 0.19, 0.14);

    // create collision detector
    object2->createAABBCollisionDetector(toolRadius);

    // add object to world
    world->addChild(object2);

    // set the position of the object
    object2->setLocalPos(-0.07, 0.15, 0.0501);

    object2->rotateAboutLocalAxisDeg (0,0,1,-20);

    // set graphic properties
    object2->m_texture = cTexture2d::create();
    fileload = object2->m_texture->loadFromFile("image_objects/grass.jpg");
    if (!fileload)
    {
        #if defined(_MSVC)
        fileload = object2->m_texture->loadFromFile("image_objects/grass.jpg");
        #endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // enable texture mapping
    object2->setUseTexture(true);
    object2->m_material->setWhite();

    // create normal map from texture data
    cNormalMapPtr normalMap2 = cNormalMap::create();
    normalMap2->createMap(object2->m_texture);
    object2->m_normalMap = normalMap2;

    // set haptic properties
    object2->m_material->setStiffness(0.2 * maxStiffness);
    object2->m_material->setStaticFriction(0.2);
    object2->m_material->setDynamicFriction(0.2);
    object2->m_material->setTextureLevel(0.075);
    object2->m_material->setHapticTriangleSides(true, false);

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();
    
    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setBlack();
    // camera->m_frontLayer->addChild(labelRates);


    buildingLabel = new cLabel(font);
    buildingLabel->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel);

    buildingLabel2 = new cLabel(font);
    buildingLabel2->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel2);

    buildingLabel3 = new cLabel(font);
    buildingLabel3->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel3);

    buildingLabel4 = new cLabel(font);
    buildingLabel4->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel4);

    buildingLabel5 = new cLabel(font);
    buildingLabel5->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel5);

    buildingLabel6 = new cLabel(font);
    buildingLabel6->m_fontColor.setWhite();
    // camera->m_frontLayer->addChild(buildingLabel6);

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(0.3, 0.3, 0.3),
                                cColorf(0.2, 0.2, 0.2),
                                cColorf(0.1, 0.1, 0.1),
                                cColorf(0.0, 0.0, 0.0));


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width  = a_width;
    height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
                        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);

    // cVector3d* pos;
    // pos = &(tool->m_hapticPoint->getGlobalPosProxy());

    buildingLabel->setText("Nymble");
    buildingLabel->setLocalPos(200,205,0);

    buildingLabel2->setText("Entre");
    buildingLabel2->setLocalPos(400,175,0);

    buildingLabel3->setText("D");
    buildingLabel3->setLocalPos(725,100,0);

    buildingLabel4->setText("E");
    buildingLabel4->setLocalPos(555,135,0);

    buildingLabel5->setText("Biblioteket");
    buildingLabel5->setLocalPos(400,410,0);

    buildingLabel6->setText("Arktektur");
    buildingLabel6->setLocalPos(425,300,0);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}

//------------------------------------------------------------------------------
enum cMode
{
    IDLE,
    SELECTION
};

void updateHaptics(void)
{
    cMode state = IDLE;
    cGenericObject* selectedObject = NULL;
    cTransform tool_T_object;

    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////////
        // HAPTIC RENDERING
        /////////////////////////////////////////////////////////////////////////

        // signal frequency counter
        freqCounterHaptics.signal(1);

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();


        /*
        /////////////////////////////////////////////////////////////////////////
        // MANIPULATION
        /////////////////////////////////////////////////////////////////////////

        // compute transformation from world to tool (haptic device)
        cTransform world_T_tool = tool->getDeviceGlobalTransform();

        // get status of user switch
        bool button = tool->getUserSwitch(0);

        //
        // STATE 1:
        // Idle mode - user presses the user switch
        //
        if ((state == IDLE) && (button == true))
        {
            // check if at least one contact has occurred
            if (tool->m_hapticPoint->getNumCollisionEvents() > 0)
            {
                // get contact event
                cCollisionEvent* collisionEvent = tool->m_hapticPoint->getCollisionEvent(0);

                // get object from contact event
                selectedObject = collisionEvent->m_object;
            }
            else
            {
                selectedObject = object;
            }

            // get transformation from object
            cTransform world_T_object = selectedObject->getGlobalTransform();

            // compute inverse transformation from contact point to object
            cTransform tool_T_world = world_T_tool;
            tool_T_world.invert();

            // store current transformation tool
            tool_T_object = tool_T_world * world_T_object;

            // update state
            state = SELECTION;
        }


        //
        // STATE 2:
        // Selection mode - operator maintains user switch enabled and moves object
        //
        else if ((state == SELECTION) && (button == true))
        {
            // compute new transformation of object in global coordinates
            cTransform world_T_object = world_T_tool * tool_T_object;

            // compute new transformation of object in local coordinates
            cTransform parent_T_world = selectedObject->getParent()->getLocalTransform();
            parent_T_world.invert();
            cTransform parent_T_object = parent_T_world * world_T_object;

            // assign new local transformation to object
            selectedObject->setLocalTransform(parent_T_object);

            // set zero forces when manipulating objects
            tool->setDeviceGlobalForce(0.0, 0.0, 0.0);

            tool->initialize();
        }

        //
        // STATE 3:
        // Finalize Selection mode - operator releases user switch.
        //
        else
        {
            state = IDLE;
        }
        */


        /////////////////////////////////////////////////////////////////////////
        // FINALIZE
        /////////////////////////////////////////////////////////////////////////

        // send forces to haptic device
        //tool->applyToDevice();
        cVector3d computedForce = tool->getDeviceGlobalForce();
        computedForce += cVector3d(0,0,-.5);
        hapticDevice->setForce(computedForce);

    }
    
    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------


//==============================================================================
/*
 *
 *  Example code borrowed from Chai3D library:

    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \version   3.2.0 $Rev: 1925 $
*/
//==============================================================================

