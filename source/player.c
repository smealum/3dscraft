#include <3ds.h>

#include "text.h"
#include "player.h"
#include "world.h"
#include "block.h"
#include "gs.h"

void initPlayer(player_s* p)
{
	if(!p)return;

	initCamera(&p->camera);
	p->position=vect3Df(0.0f, CHUNK_HEIGHT*CLUSTER_SIZE/2+CLUSTER_SIZE, 0.0f);
	loadIdentity44((float*)p->camera.orientation);
	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
	p->acceleration=vect3Df(0.0f, 0.0f, 0.0f);
	p->headbob=0.0f;
	p->flying=false;
	p->destroy=false;
	p->block=BLOCK_DIRT;

	initCursor(&p->cursor);
}

//TODO : cleanup ?
vect3Di_s performRayMarch(world_s* w, vect3Df_s localBlockPosf, vect3Df_s localBlockPosf2, vect3Df_s* out, bool* collided, u8* dirout)
{
	vect3Di_s cur=vf2i(localBlockPosf);
	vect3Df_s u=vnormf(vsubf(localBlockPosf2, localBlockPosf));
	const float d=vdistf(localBlockPosf2,localBlockPosf);
	
	if(!w)return cur;

	const int stepX=(localBlockPosf2.x>localBlockPosf.x)?1:-1;
	const int stepY=(localBlockPosf2.y>localBlockPosf.y)?1:-1;
	const int stepZ=(localBlockPosf2.z>localBlockPosf.z)?1:-1;

	const float tDeltaX=fabs(1.0f/u.x); // w/u.x
	const float tDeltaY=fabs(1.0f/u.y); // h/u.y
	const float tDeltaZ=fabs(1.0f/u.z); // z/u.z

	float tMaxX, tMaxY, tMaxZ;
	if(fabs(u.x)<0.001f)tMaxX=d;
	else tMaxX=fabs((localBlockPosf.x-floorf(localBlockPosf.x)+((localBlockPosf2.x>localBlockPosf.x)?-1.0f:0.0f))/u.x);
	if(fabs(u.y)<0.001f)tMaxY=d;
	else tMaxY=fabs((localBlockPosf.y-floorf(localBlockPosf.y)+((localBlockPosf2.y>localBlockPosf.y)?-1.0f:0.0f))/u.y);
	if(fabs(u.z)<0.001f)tMaxZ=d;
	else tMaxZ=fabs((localBlockPosf.z-floorf(localBlockPosf.z)+((localBlockPosf2.z>localBlockPosf.z)?-1.0f:0.0f))/u.z);

	if(collided)*collided=false;

	u8 dir=0;
	if(out)*out=localBlockPosf2;
	do{
		if(tMaxX>=d && tMaxY>=d && tMaxZ>=d)return cur; //finished without colliding with world
		if(tMaxX < tMaxY)
		{
			if(tMaxX < tMaxZ)
			{
				dir=0;
				cur.x+=stepX;
				tMaxX+=tDeltaX;
			}else{
				dir=2;
				cur.z+=stepZ;
				tMaxZ+=tDeltaZ;
			}   
		}else{
			if(tMaxY < tMaxZ) {
				dir=1;
				cur.y+=stepY;
				tMaxY+=tDeltaY;
			}else{
				dir=2;
				cur.z+=stepZ;
				tMaxZ+=tDeltaZ;
			}
		}
	}while(getWorldBlock(w, cur)==0);

	if(collided)*collided=true;
	if(dirout)*dirout=dir*2;

	switch(dir)
	{
		case 0:
			{
				float targetX=(cur.x)*1.0f;
				if(stepX<0)targetX+=1.0f;
				else if(dirout)(*dirout)++;
				if(out)
				{
					float r=(targetX-localBlockPosf.x)/u.x;
					targetX-=0.1f*stepX; //margin
					*out=vaddf(localBlockPosf, vmulf(u,r)); out->x=targetX;
				}
			}
			break;
		case 1:
			{
				float targetY=(cur.y)*1.0f;
				if(stepY<0)targetY+=1.0f;
				else if(dirout)(*dirout)++;
				if(out)
				{
					float r=(targetY-localBlockPosf.y)/u.y;
					targetY-=0.04f*stepY; //margin
					*out=vaddf(localBlockPosf, vmulf(u,r)); out->y=targetY;
				}
			}
			break;
		case 2:
			{
				float targetZ=(cur.z)*1.0f;
				if(stepZ<0)targetZ+=1.0f;
				else if(dirout)(*dirout)++;
				if(out)
				{
					float r=(targetZ-localBlockPosf.z)/u.z;
					targetZ-=0.1f*stepZ; //margin
					*out=vaddf(localBlockPosf, vmulf(u,r)); out->z=targetZ;
				}
			}
			break;
	}

	return cur;
}

const vect3Di_s directionVector[]=
{
	{+1,0,0},
	{-1,0,0},
	{0,+1,0},
	{0,-1,0},
	{0,0,+1},
	{0,0,-1},
};

touchPosition previousTouch;

void controlsPlayer(player_s* p, world_s* w)
{
	if(!p || !w)return;

	const u32 PAD=hidKeysHeld();
	circlePosition cpad, cstick;
	touchPosition touch;
	hidTouchRead(&touch);
	hidCircleRead(&cpad);
	hidCstickRead(&cstick);

	vect3Df_s vx=vnormf(getMatrixColumn((float*)p->camera.orientation, 0));
	vect3Df_s vy=vnormf(getMatrixColumn((float*)p->camera.orientation, 1));
	vect3Df_s vz=vnormf(getMatrixColumn((float*)p->camera.orientation, 2));
	vect3Df_s vz2=(!p->flying)?vnormf(vect3Df(vz.x,0.0f,vz.z)):vz;

	if(cpad.dy>20){p->acceleration=vaddf(p->acceleration, vmulf(vz2, -(cpad.dy*500.0f)/0x9C));if(!p->flying)p->headbob+=0.3f;}
	if(cpad.dy<-20){p->acceleration=vaddf(p->acceleration, vmulf(vz2, -(cpad.dy*500.0f)/0x9C));if(!p->flying)p->headbob-=0.3f;}
	if(cpad.dx>20){p->acceleration=vaddf(p->acceleration, vmulf(vx, (cpad.dx*500.0f)/0x9C));if(!p->flying)p->headbob+=0.3f;}
	if(cpad.dx<-20){p->acceleration=vaddf(p->acceleration, vmulf(vx, (cpad.dx*500.0f)/0x9C));if(!p->flying)p->headbob+=0.3f;}

	if(hidKeysDown()&KEY_DRIGHT)p->block++;
	else if(hidKeysDown()&KEY_DLEFT)p->block--;
	p->block%=BLOCK_NUM;
	if(!p->block)p->block++;

	if(hidKeysDown()&KEY_ZL || hidKeysDown()&KEY_DUP)p->flying^=1;
	if(hidKeysDown()&KEY_SELECT)p->destroy^=1;

	if(!p->flying)
	{
		if(hidKeysDown()&KEY_L)p->velocity=vaddf(p->velocity, vect3Df(0.0f, 35.0f, 0.0f)); //jumping acts on velocity directly
	}else{
		if(PAD&KEY_L)p->acceleration=vaddf(p->acceleration, vmulf(vy, 500.0f));
		if(PAD&KEY_R)p->acceleration=vaddf(p->acceleration, vmulf(vy, -500.0f));
	}

	const vect3Df_s v=vmulf(vz, -5.0f);
	bool collided=false;
	u8 dir;
	vect3Di_s out=performRayMarch(w, p->position, vaddf(p->position, v), NULL, &collided, &dir);
	if(collided)
	{
		p->cursor.active=true;
		p->cursor.position=out;
		p->cursor.direction=dir;
		if((!p->flying && hidKeysDown()&KEY_R) || (hidKeysDown()&KEY_DDOWN))alterWorldBlock(w, p->destroy?out:vaddi(out, directionVector[dir]), p->destroy?BLOCK_AIR:p->block, true);
		if(hidKeysDown()&KEY_ZR)alterWorldBlock(w, p->destroy?vaddi(out, directionVector[dir]):out, p->destroy?p->block:BLOCK_AIR, true);
	}else p->cursor.active=false;

	if((keysHeld()&KEY_TOUCH)&&!(keysDown()&KEY_TOUCH))
	{
		p->deltaOrientation.x+=-(touch.py-previousTouch.py)*1.5f;
		p->deltaOrientation.y+=(touch.px-previousTouch.px)*1.5f;
	}

	cstick.dx=(abs(cstick.dx)<5)?0:cstick.dx;
	cstick.dy=(abs(cstick.dy)<5)?0:cstick.dy;

	p->deltaOrientation.x+=(cstick.dy*0.07f)/2.6f;
	p->deltaOrientation.y+=(cstick.dx*0.07f)/2.6f;
	
	previousTouch=touch;
}

extern u32 debugValue[];

#define PLAYERSIZE (0.25f)
#define PLAYERHEIGHT (0.4f)
vect3Df_s playerBox[]={(vect3Df_s){-PLAYERSIZE,-PLAYERHEIGHT*3,-PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,-PLAYERHEIGHT*3,-PLAYERSIZE},
						(vect3Df_s){-PLAYERSIZE,-PLAYERHEIGHT*3,+PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,-PLAYERHEIGHT*3,+PLAYERSIZE},

						(vect3Df_s){-PLAYERSIZE,0.0f,-PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,0.0f,-PLAYERSIZE},
						(vect3Df_s){-PLAYERSIZE,0.0f,+PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,0.0f,+PLAYERSIZE},

						(vect3Df_s){-PLAYERSIZE,+PLAYERHEIGHT,-PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,+PLAYERHEIGHT,-PLAYERSIZE},
						(vect3Df_s){-PLAYERSIZE,+PLAYERHEIGHT,+PLAYERSIZE},
						(vect3Df_s){+PLAYERSIZE,+PLAYERHEIGHT,+PLAYERSIZE}};

void updatePlayer(player_s* p, world_s* w, float timeDelta)
{
	if(!p)return;

	const float dt=1.0f/60;
	float v;

	//gravity
	if(!p->flying)p->acceleration=vaddf(p->acceleration, vect3Df(0.0f, -100.0f, 0.0f));

	for(v=0.0f; timeDelta-v > 0.001f; v+=dt)
	{
		p->velocity=vaddf(p->velocity, vmulf(p->acceleration, dt));
		p->velocity=vaddf(vmulf(p->velocity, dt), vmulf(p->acceleration, dt*dt/2));

		//collisions
		if(vmagf(p->velocity)>0.0001f)
		{
			vect3Df_s v=p->velocity;
			int i;
			for(i=0; i<12; i++)
			{
				vect3Df_s out;
				vect3Df_s pt=vaddf(p->position, playerBox[i]);
				performRayMarch(w, pt, vaddf(pt, v), &out, NULL, NULL);
				v=vsubf(out,pt);
				if(vmagf(v)<=0.0001f)break;
			}
			p->velocity=v;
		}

		p->position=vaddf(p->position, p->velocity);
		p->camera.position=vaddf(p->position,vect3Df(0.0f, cos(p->headbob)*0.05f, 0.0f));

		if(!p->flying)p->velocity=vect3Df(0.0f, p->velocity.y/dt, 0.0f);
		else p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
	}

	rotateMatrixX((float*)p->camera.orientation, p->deltaOrientation.x*timeDelta, true);
	rotateMatrixY((float*)p->camera.orientation, p->deltaOrientation.y*timeDelta, false);

	p->acceleration=vect3Df(0.0f, 0.0f, 0.0f);
	p->deltaOrientation=vect3Df(0.0f, 0.0f, 0.0f);

	//world streaming
	if(w)
	{
		vect3Di_s off=vsubi(vf2i(vmulf(p->position, 1.0f/CLUSTER_SIZE)), vaddi(w->position, vect3Di(WORLD_SIZE/2,0,WORLD_SIZE/2)));
		if(off.x<=-2)translateWorld(w, vect3Di(-1,0,0));
		if(off.x>=2)translateWorld(w, vect3Di(1,0,0));
		if(off.z<=-2)translateWorld(w, vect3Di(0,0,-1));
		if(off.z>=2)translateWorld(w, vect3Di(0,0,1));
	}

	updateCamera(&p->camera);
}

void setCameraPlayer(player_s* p)
{
	if(!p)return;

	setCamera(&p->camera);
}
