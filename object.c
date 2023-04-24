#include <stdlib.h>
#include "object.h"
#include "memory.h"

void AddUser(Object *obj){
    obj->numUsers++;
 }

void RemoveUser(Object *obj){
    obj->numUsers--;
    if(obj->numUsers == 0)
    	Object_Free(obj);
 }

Object *Object_Create(){
	Object *obj =  malloc(sizeof(Object));
	memset(obj, 0 , sizeof(Object));
	obj->RemoveUser = RemoveUser;
	obj->AddUser = AddUser;
 }

Object *Object_Copy(Object *obj){
	Object *obj2 = malloc(sizeof(Object));
	memcpy(obj2, obj, sizeof(Object));
	return obj2;
}

void Object_Free(Object *obj){
	free(obj);
}
void Object_Freeze(Object *obj){}
Object *Object_Set(Object *ret){}
