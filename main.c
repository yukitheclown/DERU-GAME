#include <GL/glew.h>
#include "window.h"
#include "thoth/thoth.h"
#include "sound.h"
#include "world.h"
#include "mesh.h"
#include "image_loader.h"
#include "shaders.h"
#include "object.h"
// #include "skybox.h"
#define WINDOW_WIDTH 480*2 
#define WINDOW_HEIGHT 272*2
#define NOTE_RANGE 1.5

static Object *nepObj, *cubeObj;
static Animation cubeAnim;
static Skeleton cubeSkel;
static PlayingAnimation cubeAnims[1];
// static Thoth_t *thoth;

// static Skybox skybox;

static float mouseSensitivity = 0.001;
static float moveSpeed = 0.005;

static Vec2 rotation = {0,0};
static Vec3 position = {0,0,10};

static char movingDirs[4];
static Image test;

static void SetObjectsPos(Object *obj, Vec3 pos, Vec3 rotation);

float GetDeltaTime(void){
    return Window_GetDeltaTime();
}

static void Update(){

    cubeAnims[0].into += Window_GetDeltaTime() / 10.1f;

    if(cubeAnims[0].into >= cubeAnim.length)
        cubeAnims[0].into = 0;

	Skeleton_Update(&cubeSkel, cubeAnims, 1);

	Vec3 moveVec = {0,0,0};

	if(movingDirs[0]) moveVec.z -= 1;
	if(movingDirs[1]) moveVec.z += 1;
	if(movingDirs[2]) moveVec.x -= 1;
	if(movingDirs[3]) moveVec.x += 1;
    
    if(Math_Vec3Magnitude(moveVec)){

        moveVec = Math_Vec3Normalize(moveVec);

        moveVec = Math_Rotate(moveVec, (Vec3){-rotation.y, -rotation.x, 0});

        moveVec = Math_Vec3MultFloat(moveVec, Window_GetDeltaTime() * moveSpeed);

        position.x += moveVec.x;
        position.z += moveVec.z;
    }
}

static void Event(SDL_Event ev){

    // Thoth_Event(thoth, ev);

    // if(ev.window.event == SDL_WINDOWEVENT_RESIZED || 
    //     ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
    //     int w = ev.window.data1;
    //     int h = ev.window.data2;

    //     Thoth_Resize(thoth, w/4, 0, (w/2) + (w/4), h);
    // }

	if(ev.type == SDL_MOUSEMOTION){

		rotation.x += mouseSensitivity * ev.motion.xrel;
		rotation.y += mouseSensitivity * ev.motion.yrel;

        while(rotation.x > PI*2) rotation.x -= PI*2;
        while(rotation.x < 0) rotation.x += PI*2;

        if(rotation.y < -PI/2.5) rotation.y = -PI/2.5;
        if(rotation.y > PI/3) rotation.y = PI/3;

	} else if(ev.type == SDL_KEYDOWN){

        if(ev.key.keysym.sym == SDLK_w)
            movingDirs[0] = 1;
		else if(ev.key.keysym.sym == SDLK_s)
			movingDirs[1] = 1;
		else if(ev.key.keysym.sym == SDLK_a)
			movingDirs[2] = 1;
        else if(ev.key.keysym.sym == SDLK_d)
            movingDirs[3] = 1;
        else if(ev.key.keysym.sym == SDLK_ESCAPE)
            exit(0);

	} else if(ev.type == SDL_KEYUP){

		if(ev.key.keysym.sym == SDLK_w)
			movingDirs[0] = 0;
		else if(ev.key.keysym.sym == SDLK_s)
			movingDirs[1] = 0;
		else if(ev.key.keysym.sym == SDLK_a)
			movingDirs[2] = 0;
		else if(ev.key.keysym.sym == SDLK_d)
			movingDirs[3] = 0;

        int k;
        for(k = 0; k < 4; k++) if(movingDirs[k] != 0) break;

	}
}

static void Focus(){

} 


static void DrawRigged(Object *obj){

    int currProgram = Shaders_GetProgram(SKELETAL_ANIMATION_SHADER);
   
    Shaders_SetModelMatrix(obj->matrix);
    Shaders_UpdateModelMatrix();

    // Skeleton_Update(&cubeSkel, cubeAnims, 1);
    
    
    Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
    glUniform4fv(Shaders_GetBonesLocation(), cubeSkel.nBones * 3, &cubeSkel.matrices[0].x);


    Shaders_SetModelMatrix(obj->matrix);
    Shaders_UpdateModelMatrix();

    glActiveTexture(GL_TEXTURE0);
    
    glBindVertexArray(obj->model.vao);

    int curr = 0;


    int k;
    for(k = 0; k < obj->model.nMaterials; k++){

        glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].texture);

        glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
        curr += obj->model.nElements[k];
    }

    glBindVertexArray(0);
}
static void DrawModel(Object *obj){

    int currProgram = Shaders_GetProgram(TEXTURED_SHADER);

    
    Shaders_UseProgram(TEXTURED_SHADER);

    Shaders_SetModelMatrix(obj->matrix);
    Shaders_UpdateModelMatrix();
    // DrawModel();

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(obj->model.vao);

    int curr = 0;


    int k;
    for(k = 0; k < obj->model.nMaterials; k++){
        glBindTexture(GL_TEXTURE_2D, obj->model.materials[k].texture);
        glDrawElements(GL_TRIANGLES, obj->model.nElements[k], GL_UNSIGNED_INT, (void *)(curr * sizeof(GLuint)));
        curr += obj->model.nElements[k];
    }

    glBindVertexArray(0);
}
static char Draw(){
    
    
    Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
    // Shaders_UseProgram(TEXTURED_SHADER);

    float persp[16];

    Vec3 forward = Math_Rotate((Vec3){0,0,-1}, (Vec3){-rotation.y, -rotation.x, 0});
    float view[16];
    Math_LookAt(view, position, Math_Vec3AddVec3(position, forward), (Vec3){0,1,0});
    Shaders_SetViewMatrix(view);
    Shaders_UpdateViewMatrix(view);

    Math_Perspective(persp, 60.0f*(3.1415/180), (float)1920 / (float)1080, 0.1f, 50.0f);
    Shaders_SetProjectionMatrix(persp);
    Shaders_UpdateProjectionMatrix();


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Math_Identity(cubeObj->matrix);
    Shaders_SetModelMatrix(cubeObj->matrix);
    Shaders_UpdateModelMatrix();

    glCullFace(GL_BACK);

    // World_DrawOctree();
    World_Render(1);

    // Thoth_Render(thoth); //stencil buffer/framebuffer access todo
    Vec3 lines[18]; 
    int k;
    for(k = 0; k < cubeSkel.nBones; k++){
        lines[0] = (Vec3){cubeSkel.bones[k].points[0].x, cubeSkel.bones[k].points[0].y, cubeSkel.bones[k].points[0].z};
        lines[1] = (Vec3){cubeSkel.bones[k].points[1].x, cubeSkel.bones[k].points[1].y, cubeSkel.bones[k].points[1].z};
        lines[2] = (Vec3){cubeSkel.bones[k].points[2].x, cubeSkel.bones[k].points[2].y, cubeSkel.bones[k].points[2].z};
        lines[3] = (Vec3){cubeSkel.bones[k].points[3].x, cubeSkel.bones[k].points[3].y, cubeSkel.bones[k].points[3].z};
        lines[4] = (Vec3){cubeSkel.bones[k].points[0].x, cubeSkel.bones[k].points[0].y, cubeSkel.bones[k].points[0].z};
        lines[5] = (Vec3){cubeSkel.bones[k].points[4].x, cubeSkel.bones[k].points[4].y, cubeSkel.bones[k].points[4].z};
        lines[6] = (Vec3){cubeSkel.bones[k].points[5].x, cubeSkel.bones[k].points[5].y, cubeSkel.bones[k].points[5].z};
        lines[7] = (Vec3){cubeSkel.bones[k].points[6].x, cubeSkel.bones[k].points[6].y, cubeSkel.bones[k].points[6].z};
        lines[8] = (Vec3){cubeSkel.bones[k].points[7].x, cubeSkel.bones[k].points[7].y, cubeSkel.bones[k].points[7].z};
        lines[9] = (Vec3){cubeSkel.bones[k].points[4].x, cubeSkel.bones[k].points[4].y, cubeSkel.bones[k].points[4].z};
        lines[10] = (Vec3){cubeSkel.bones[k].points[7].x, cubeSkel.bones[k].points[7].y, cubeSkel.bones[k].points[7].z};
        lines[11] = (Vec3){cubeSkel.bones[k].points[3].x, cubeSkel.bones[k].points[3].y, cubeSkel.bones[k].points[3].z};
        lines[12] = (Vec3){cubeSkel.bones[k].points[7].x, cubeSkel.bones[k].points[7].y, cubeSkel.bones[k].points[7].z};
        lines[13] = (Vec3){cubeSkel.bones[k].points[6].x, cubeSkel.bones[k].points[6].y, cubeSkel.bones[k].points[6].z};
        lines[14] = (Vec3){cubeSkel.bones[k].points[2].x, cubeSkel.bones[k].points[2].y, cubeSkel.bones[k].points[2].z};
        lines[15] = (Vec3){cubeSkel.bones[k].points[6].x, cubeSkel.bones[k].points[6].y, cubeSkel.bones[k].points[6].z};
        lines[16] = (Vec3){cubeSkel.bones[k].points[5].x, cubeSkel.bones[k].points[5].y, cubeSkel.bones[k].points[5].z};
        lines[17] = (Vec3){cubeSkel.bones[k].points[1].x, cubeSkel.bones[k].points[1].y, cubeSkel.bones[k].points[1].z};
    
        glLineWidth(3);

        Vec3 axes[3] = {(Vec3){1,0,0}, (Vec3){0,1,0}, (Vec3){0,0,1}};

        Vec3 points[8];    

        static float thrown = 0;
        thrown += GetDeltaTime() / 2000.0f;
        if(thrown > 10) thrown = 0;

        Cube cube = (Cube){position.x,position.y,position.z,0.5,0.5,0.5};
        cube.x += thrown * forward.x;
        cube.y += thrown * forward.y;
        cube.z += thrown * forward.z;
        points[0] = (Vec3){cube.x, cube.y, cube.z};
        points[1] = (Vec3){cube.x+cube.w, cube.y, cube.z};
        points[2] = (Vec3){cube.x+cube.w, cube.y+cube.h, cube.z};
        points[3] = (Vec3){cube.x, cube.y+cube.h, cube.z};
        points[4] = (Vec3){cube.x, cube.y, cube.z+cube.d};
        points[5] = (Vec3){cube.x+cube.w, cube.y, cube.z+cube.d};
        points[6] = (Vec3){cube.x+cube.w, cube.y+cube.h, cube.z+cube.d};
        points[7] = (Vec3){cube.x, cube.y+cube.h, cube.z+cube.d};

        if(SAT_Collision(cubeSkel.bones[k].points, 8, points, 8, cubeSkel.bones[k].axes, axes) > 0){
            World_DrawLines(lines, 18);
        }

        lines[0] = cubeSkel.bones[k].points[0];
        lines[1] = Math_Vec3AddVec3(cubeSkel.bones[k].points[0], cubeSkel.bones[k].axes[0]);
        lines[2] = cubeSkel.bones[k].points[0];
        lines[3] = Math_Vec3AddVec3(cubeSkel.bones[k].points[0], cubeSkel.bones[k].axes[1]);
        lines[4] = cubeSkel.bones[k].points[0];
        lines[5] = Math_Vec3AddVec3(cubeSkel.bones[k].points[0], cubeSkel.bones[k].axes[2]);
        World_DrawLines(lines, 6);


        lines[0] = (Vec3){points[0].x, points[0].y, points[0].z};
        lines[1] = (Vec3){points[1].x, points[1].y, points[1].z};
        lines[2] = (Vec3){points[2].x, points[2].y, points[2].z};
        lines[3] = (Vec3){points[3].x, points[3].y, points[3].z};
        lines[4] = (Vec3){points[0].x, points[0].y, points[0].z};
        lines[5] = (Vec3){points[4].x, points[4].y, points[4].z};
        lines[6] = (Vec3){points[5].x, points[5].y, points[5].z};
        lines[7] = (Vec3){points[6].x, points[6].y, points[6].z};
        lines[8] = (Vec3){points[7].x, points[7].y, points[7].z};
        lines[9] = (Vec3){points[4].x, points[4].y, points[4].z};
        lines[10] = (Vec3){points[7].x, points[7].y, points[7].z};
        lines[11] = (Vec3){points[3].x, points[3].y, points[3].z};
        lines[12] = (Vec3){points[7].x, points[7].y, points[7].z};
        lines[13] = (Vec3){points[6].x, points[6].y, points[6].z};
        lines[14] = (Vec3){points[2].x, points[2].y, points[2].z};
        lines[15] = (Vec3){points[6].x, points[6].y, points[6].z};
        lines[16] = (Vec3){points[5].x, points[5].y, points[5].z};
        lines[17] = (Vec3){points[1].x, points[1].y, points[1].z};
        World_DrawLines(lines, 18);


    }



    return 1;
}

static void OnResize(){
        // Thoth_Render(thoth); //stencil buffer/framebuffer access todo

}

static void SetObjectsPos(Object *obj, Vec3 pos, Vec3 rotation){

    obj->SetPosition(obj, pos);
    obj->Rotate(obj, rotation);
    obj->UpdateMatrix(obj);

    obj->AddUser(obj);
    World_UpdateObjectInOctree(obj);
    obj->RemoveUser(obj);
}


int main(int argc, char **argv){


    Window_Open("Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH, WINDOW_HEIGHT, 0);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    Memory_Init((0x01 << 20) * 64);

    glClearColor(0,0,0,1);
    Shaders_Init();

    World_InitOctree((Vec3){-100, -100, -100}, 200, 25);

    glClearColor(0,0,0,1);


    float persp[16], view[16], model[16];
    Math_Perspective(persp, 60.0f*(3.1415/180), (float)1920 / (float)1080, 0.1f, 100.0f);
    Math_LookAt(view, (Vec3){0,0,-5}, (Vec3){0,0,0}, (Vec3){0,1,0});
    Math_Identity(model);
    Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
    Shaders_SetProjectionMatrix(persp);
    Shaders_UpdateProjectionMatrix();
    Shaders_SetModelMatrix(model);
    Shaders_UpdateModelMatrix();
    Shaders_SetViewMatrix(view);
    Shaders_UpdateViewMatrix(view);
    Shaders_UpdateProjectionMatrix();
    memset(&cubeSkel, 0, sizeof(Skeleton));
    cubeObj = Object_Create();
   cubeObj->skeleton = &cubeSkel;
   memcpy(cubeObj->matrix, Math_Identity, sizeof(Math_Identity));
    RiggedModel_Load(&cubeObj->model, &cubeSkel, "Resources\\monkey.yuk");
    Model_LoadCollisions(&cubeObj->model, "Resources\\test.col");
    // Model_Load(&cubeObj->model, "Resources\\untitled_ArmatureAction.anm");
    memset(&cubeAnim, 0, sizeof(Animation));
    Animation_Load(&cubeAnim, "Resources\\monkey_ArmatureAction.anm");
    test = ImageLoader_CreateImage("Resources\\test.png", 1);

    memset(&cubeObj->bb, 0, sizeof(BoundingBox));
    cubeObj->bb.cube = (Cube){cubeObj->model.bb[0].x,cubeObj->model.bb[0].y,cubeObj->model.bb[0].z,
        cubeObj->model.bb[0].w,cubeObj->model.bb[0].h, cubeObj->model.bb[0].d};
    cubeObj->bb.pos = (Vec3){cubeObj->matrix[0], cubeObj->matrix[5],cubeObj->matrix[11]};
    
    int k;
    for(k = 1; k < cubeObj->model.numBB; k++){
        BoundingBox child;
        memset(&child, 0, sizeof(BoundingBox));
        child.cube = (Cube){cubeObj->model.bb[k].x,cubeObj->model.bb[k].y,cubeObj->model.bb[k].z,
            cubeObj->model.bb[k].w,cubeObj->model.bb[k].h, cubeObj->model.bb[k].d};
        child.pos = (Vec3){cubeObj->matrix[0], cubeObj->matrix[5],cubeObj->matrix[11]};
        BoundingBox_AddChild(&cubeObj->bb,  &child);
    }


    cubeObj->Draw = DrawRigged;
    cubeObj->AddUser(cubeObj);
    World_UpdateObjectInOctree(cubeObj);


    cubeAnims[0] = (PlayingAnimation){
        .active = 1,
        .weight = 1,
        .into = 0,
        .anim = &cubeAnim,
    };

    // thoth = Thoth_Create(WINDOW_WIDTH, WINDOW_HEIGHT );
    // Thoth_LoadFile(thoth, "main.c");
    // Thoth_Resize(thoth, 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

    Window_MainLoop(Update, Event, Draw, Focus, OnResize, 1, 1);


    // Thoth_Destroy(thoth);

	World_Free();
    Shaders_Close();
    ImageLoader_Free();


    return 0;
}