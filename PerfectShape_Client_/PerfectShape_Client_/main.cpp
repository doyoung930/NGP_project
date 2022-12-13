#include <iostream>
#include "network.h"
#include "../../PerfectShape_Server/protocol.h"
#include "bullet.h"
#include "glew.h" //--- 필요한 헤더파일 include
#include "freeglut.h"
#include "freeglut_ext.h"
#include "glm/glm/glm.hpp"
#include "glm/glm/ext.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include <cmath>
#include <time.h>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // << 위 define과 붙어 있어한다 함. 아니면 오류남(원인 모름)

#define shp_bullet_num 20


typedef struct m {
	// 마우스 시점 제어
	float previous_y;
	float previous_x;
	float x;
	float y;

	float LR;
	float UD;
	float dpi;

	glm::vec3 Direction;

}MouseCon;

typedef struct p {
	// 플레이어
	int FB_dir; // 앞 뒤 방향 판단 변수
	int LR_dir; // 왼쪽 오른쪽 판단 변수
	glm::vec3 t; //  플레이어 위치
	glm::vec3 s; // 크기
	float speed; // 속도

	glm::vec3 hit_vector;
	float hit_speed;
	int hit_cnt;
	int hit_flag;
	int hp;

	// 플레이어 총알
	bool bullet_shot[shp_bullet_num];
	glm::vec3 bullet_t[shp_bullet_num]; // 총알
	glm::vec3 bullet_s[shp_bullet_num];
	glm::vec3 bullet_vector[shp_bullet_num];
	float bullet_speed[shp_bullet_num];

	int room;
}Player;

typedef struct Screen {
	// 화면 전환
	bool states[5]; // index 0: 메인, 1: 인게임, 2:일시정지 
};

typedef struct Map {

	glm::vec3 s;
	glm::vec3 floor_t;
	glm::vec3 roof_t;

	glm::vec3 pillar_s;
	glm::vec3 pillar_t[4][10];
	glm::vec3 pillar_normal[4];

	glm::vec3 rooms_s;
	glm::vec3 rooms_t[4];

	int open[4];
	bool wave;
	bool close;
	int clear_num;

	float light_rotate_deg;
	float LightColor[3];

	int total_map[10][10];

	float Back_ground_num;
	float Peace_num;

}MAP;

typedef struct particle {

	glm::vec3 t;
	glm::vec3 s;
	glm::vec3 vector;
	glm::vec3 p_vector;

	float speed;

	float time;
	int bounce_time;

}Particle;

typedef struct enemy {

	bool in_room;
	int kind;
	int hp;

	glm::vec3 vector_knockback;
	float knockback_speed;

	glm::vec3 view;
	glm::vec3 up;
	float ry{};

	glm::vec3 vector; //이동 방향 혹은
	glm::vec3 t; //  적 위치
	glm::vec3 s; // 크기
	float radius;// 바운딩 스피어의 반지름
	float speed; // 속도

	GLUquadricObj* bullet;
	glm::vec3 bullet_t;
	bool shot;

	Particle particles[3][30];
	bool particle_pop[3];

}ENEMY;

void InitVariable();
void InitShader();
void InitBuffer();
void InitTexture();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid TimerFunction(int value);
char* filetobuf(char* file);
GLvoid PassiveMotion(int x, int y);
GLvoid MouseEntry(int state);
GLvoid Motion(int x, int y);
GLvoid PassiveMotion(int x, int y);
void CalculateMouseDelta(int x, int y);
bool make_vertexShader();
bool make_fragmentShader();
GLvoid GenShpere(GLUquadricObj* obj, bool shpere, float r, int detail);
bool collide_sphere(glm::vec3 a, glm::vec3 b, float coll_dist);
bool collide_box(glm::vec3 bb, glm::vec3 tb, glm::vec3 bb_scale, glm::vec3 tb_scale);
float CalculateRotate(glm::vec3 n, glm::vec3 t, bool normal_z);
glm::vec3 CC_CalculateRVector(glm::vec3 input, glm::vec3 normal);
void RecvEnemyInfo();
int send_dead_packet();
void Timer_Enemy_Actions();

unsigned int Texture_Wall[3];
unsigned int Texture_Floor;
unsigned int Texture_Particle;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(-200, 200);

GLfloat Hexa[36][3] = {
{ -0.5 ,-0.5 , 0.5 }, { 0.5 ,-0.5 , 0.5 },{ -0.5 ,0.5 , 0.5 }, // 앞,뒷 면
{ 0.5 ,-0.5 , 0.5 }, { 0.5 ,0.5 , 0.5 },{ -0.5 ,0.5 , 0.5 },

{ -0.5 ,-0.5 , -0.5 }, { -0.5 ,0.5 , -0.5 },{ 0.5, -0.5 , -0.5 },
{ 0.5 ,-0.5 , -0.5 }, { -0.5 ,0.5 , -0.5 },{ 0.5 ,0.5 , -0.5 },

{ -0.5 ,0.5 , 0.5 }, { -0.5 ,0.5 , -0.5 },{ -0.5 ,-0.5 , -0.5 }, // 왼,오른쪽 면
{ -0.5 ,-0.5 , -0.5 }, { -0.5 ,-0.5 , 0.5 },{ -0.5 ,0.5 , 0.5 },

{ 0.5 ,0.5 , 0.5 }, { 0.5 ,-0.5 , 0.5 },{ 0.5 ,0.5 , -0.5 },
{ 0.5 ,-0.5 , 0.5 }, { 0.5 ,-0.5 , -0.5 },{ 0.5 ,0.5 , -0.5 },

{ -0.5 ,-0.5 , 0.5 }, { -0.5 ,-0.5 , -0.5 },{ 0.5 ,-0.5 , 0.5 }, // 위, 아래 면
{ -0.5 ,-0.5 , -0.5 }, { 0.5 ,-0.5 , -0.5 },{ 0.5 ,-0.5 , 0.5 },

{ -0.5 ,0.5 ,-0.5 }, { -0.5 ,0.5 , 0.5 },{ 0.5 ,0.5 , 0.5 },
{ 0.5 ,0.5 , -0.5 }, { -0.5 ,0.5 , -0.5 },{ 0.5 ,0.5 , 0.5 },
};
GLfloat Hexa_normal[36][3] = {
{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },
{ 0.0f, 0.0f, 1.0f }, { -0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },

{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, -1.0f},
{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f},{ 0.0f, 0.0f, -1.0f },

{ -1.0f, 0.0f, 0.0f }, {  -1.0f, 0.0f, 0.0f},{ -1.0f, 0.0f, 0.0f },
{  -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f},{ -1.0f, 0.0f, 0.0f },

{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f},
{1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f},{ 1.0f, 0.0f, 0.0f },

{  0.0f, 1.0f, 0.0f}, {  0.0f, 1.0f, 0.0f },{  0.0f, 1.0f, 0.0f},
{ 0.0f, 1.0f, 0.0f}, {  0.0f, 1.0f, 0.0f },{  0.0f, 1.0f, 0.0f },

{  0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },{  0.0f, -1.0f, 0.0f },
{  0.0f, -1.0f, 0.0f }, {  0.0f, -1.0f, 0.0f },{  0.0f, -1.0f, 0.0f },
};
GLfloat Hexa_texel[36][2] = {
	{0,0},{1,0},{0,1},
	{1,0},{1,1},{0,1},

	{0,0},{0,1},{1,0},
	{1,0},{0,1},{1,1},

	{1,1},{1,0},{0,0},
	{0,0},{0,1},{1,1},

	{1,1},{0,1},{1,0},
	{0,1},{0,0},{1,0},

	{0,0},{0,1},{1,0},
	{0,1},{1,1},{1,0},

	{0,0},{0,1},{1,1},
	{1,0},{0,0},{1,1}
};

GLfloat Rect[6][3] = {
{ -0.5 ,0.5 ,0.0 }, { -0.5 ,-0.5, 0.0 },{ 0.5 ,0.5  ,0.0}, // 앞,뒷 면
{ 0.5 ,0.5 , 0.0 }, { -0.5 ,-0.5, 0.0 },{ 0.5 ,-0.5 , 0.0},
};
GLfloat Rect_normal[6][3] = {
{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },
{ 0.0f, 0.0f, 1.0f }, { -0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },
};
GLfloat Rect_texel[6][2] = {
	{0,1},{0,0},{1,1},
	{1,1},{0,0},{1,0},
};
GLfloat crosshair[4][3] = {
	{-0.02,0.0,0.0},{0.02,0.0,0.0},
	{0.0,-0.02,0.0},{0.0,0.02,0.0},
};

GLchar* vertexsource, * fragmentsource;
GLuint vertexshader, fragmentshader;

int timefunc_flag;
int width = GetSystemMetrics(SM_CXSCREEN);
int height = GetSystemMetrics(SM_CYSCREEN);

MouseCon view_control;
Player player;
MAP map;
ENEMY enemy[20];

GLuint vao_map, vbo_map[3];
GLuint vao_enemy, vbo_enemy[2];
GLuint vao_cross, vbo_cross;
GLuint vao_particle, vbo_particle[2];
GLUquadricObj* bullet_qobj[shp_bullet_num];

Bullet NetBullets[MAX_BULLET_NUM];
GLUquadricObj* NetBullet_qobj;

GLuint s_program;

clock_t start;

int myID;
struct NetPlayer {
	int id;
	int hp = 3 ;
	GLUquadricObj* ball;
	glm::vec3 t;
	bool active = true;
};

NetPlayer NPlayers[3];

void InitVariable()
{
	start = clock();
	//플레이어

	myID = GetMyPlayerID();

	if (myID == 0)
		player.t = { 3.0f,0,3.0f };

	else if (myID == 1)
		player.t = { -3.0f,0,3.0f };

	else if (myID == 2)
		player.t = { -3.0f,0, -3.0f };

	for (int i = 0; i < 3; ++i) {
		NPlayers[i].id = i;
		
		if(NPlayers[i].id == myID)
			NPlayers[i].active = false;
		else
			NPlayers[i].active = true;

		if (NPlayers[i].id == 0)
			NPlayers[i].t = { 3.0f,-0.15,3.0f };

		else if (NPlayers[i].id == 1)
			NPlayers[i].t = { -3.0f,-0.15,3.0f };

		else if (NPlayers[i].id == 2)
			NPlayers[i].t = { -3.0f,-0.15, -3.0f };
	}

	player.speed = 0.05;
	player.FB_dir = 0;
	player.LR_dir = 0;
	player.room = -1;
	player.hit_speed = 0.2f; player.hit_flag = 0; player.hit_cnt = 0;
	player.hp = 3;
	map.wave = true;
	// 플레이어 총알
	for (int i = 0; i < shp_bullet_num; i++)
	{
		player.bullet_shot[i] = false;
		player.bullet_s[i] = { 0.125,0.125 ,0.125 };
		player.bullet_speed[i] = 0.3;
	}
	//바닥,천장과 기둥 초기화
	{
		map.s = { 10.0f,0.1f,10.0f };
		map.floor_t = { 0.0f,-0.5f,0.0f }; // 중간 방
		map.roof_t = { 0.0f,5.0f,0.0f }; // 중간 방

		map.pillar_s = { 1.0f,6.0f,1.0f };
		float dy = (map.pillar_s.y - 0.5f) * 0.5f - 0.5f;

		map.light_rotate_deg = 0;

		map.pillar_normal[0] = { 1.0f,0.0f,0.0f };
		map.pillar_normal[1] = { -1.0f,0.0f,0.0f };
		map.pillar_normal[2] = { 0.0f,0.0f,1.0f };
		map.pillar_normal[3] = { 0.0f,0.0f,-1.0f };
		for (int i = 0; i < 10; i++)
		{
			map.pillar_t[0][i] = { -map.s.x * 0.5f,dy,-map.s.z * 0.5f + 1.0f * (float)i };
			map.pillar_t[1][i] = { map.s.x * 0.5f,dy,-map.s.z * 0.5f + 1.0f * (float)i };

			map.pillar_t[2][i] = { -map.s.x * 0.5f + 1.0f * (float)i,dy,-map.s.z * 0.5f };
			map.pillar_t[3][i] = { -map.s.x * 0.5f + 1.0f * (float)i,dy,map.s.z * 0.5f };

			if (i < 4) map.open[i] = 0;
		}

		map.rooms_s = { 10.0f,5.5f,10.0f };
		map.rooms_t[0] = { -map.s.x * 1.0f,dy - 0.05f,0.0f }; // 왼
		map.rooms_t[1] = { map.s.x * 1.0f,dy - 0.05f,0.0f }; // 오
		map.rooms_t[2] = { 0.0f,dy - 0.05f,-map.s.z * 1.0f }; // 위
		map.rooms_t[3] = { 0.0f,dy - 0.05f,map.s.z * 1.0f }; // 아래
	}

	map.clear_num = 0;
}
GLvoid drawScene()
{
	glViewport(0, 0, 900, 900);
	float bg = 0;
	glClearColor(bg, bg, bg, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glLineWidth(3);
	// --------------glsl 위치값 가져오기----------------
	unsigned int viewLocation = glGetUniformLocation(s_program, "viewTransform"); // 변환
	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");

	unsigned int lightPosLocation = glGetUniformLocation(s_program, "lightPos"); //조명
	unsigned int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");
	unsigned int viewposLocation = glGetUniformLocation(s_program, "viewPos");

	unsigned int FragKindLocation = glGetUniformLocation(s_program, "fragKind"); // 프레그먼트의 종류 0.0f: 조명 X || 1.0f: 조명 O
	unsigned int ambientLightColorLocation = glGetUniformLocation(s_program, "ambientLightColor");

	glUniform3f(ambientLightColorLocation, 0.9, 0.9, 0.9);
	glUniform3f(lightColorLocation, map.LightColor[0], map.LightColor[1], map.LightColor[2]);
	glUniform3f(objColorLocation, 0.0, 0.5, 0.5);
	glUniform3f(viewposLocation, player.t.x, player.t.y, player.t.z);

	glUniform3f(lightPosLocation, 2.0f * cos(3.141592 * map.light_rotate_deg / 180.0f), 2.5f, 2.0f * sin(3.141592 * map.light_rotate_deg / 180.0f));
	map.light_rotate_deg += 2.0f;
	// -----------------뷰 행렬-----------------
	glm::vec3 cameraPos = player.t;
	glm::vec3 cameraDirection = view_control.Direction + cameraPos;
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	// ---------------투영 행렬-------------------
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 80.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, 0.0));
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	// ---------------모델 행렬들-------------------
	glm::mat4 all = glm::mat4(1.0f);
	glm::mat4 S_field = glm::mat4(1.0f);
	glm::mat4 S_pillar = glm::mat4(1.0f);
	glm::mat4 S_rooms = glm::mat4(1.0f);

	S_field = scale(S_field, map.s);
	S_pillar = scale(S_pillar, map.pillar_s);
	S_rooms = scale(S_rooms, map.rooms_s);
	//--------------------그리기---------------------
	glUniform3f(objColorLocation, 0.3, 0.3, 0.3);
	glUniform1f(FragKindLocation, 1.0f);
	//중간 방 바닥,천장
	glFrontFace(GL_CW);
	glm::mat4 T_field = glm::mat4(1.0f);
	T_field = translate(T_field, map.floor_t);
	all = T_field * S_field;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
	glBindVertexArray(vao_map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture_Floor);
	glDrawArrays(GL_TRIANGLES, 24, 6);

	T_field = glm::mat4(1.0f);
	T_field = translate(T_field, map.roof_t);
	all = T_field * S_field;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
	glBindVertexArray(vao_map);
	glBindTexture(GL_TEXTURE_2D, Texture_Wall[2]);
	glDrawArrays(GL_TRIANGLES, 30, 6);
	glFrontFace(GL_CCW);
	
	//중간 방 기둥
	int pillar_id = GetPillarID();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (i == pillar_id) {
				if(j == 4 || j == 5)
					map.pillar_t[pillar_id][j].y = GetPillarY();
			}
			glm::mat4 T_pillar = glm::mat4(1.0f);
			T_pillar = translate(T_pillar, map.pillar_t[i][j]);
			all = T_pillar * S_pillar;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			glBindVertexArray(vao_map);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	for (int i = 0; i < 3; ++i) {
		//NPlayers[i].active = GetPlayerState(i);
		NPlayers[i].hp = GetPlayerHp(i);
		//if (NPlayers[i].active) {
		if (NPlayers[i].hp > 0){
			if (i == 0) {
				glUniform3f(objColorLocation, 0.95, 0.93, 0.4);
			}
			if (i == 1) {
				glUniform3f(objColorLocation, 0.66, 0.25, 1.0);
			}
			if (i == 2) {
				glUniform3f(objColorLocation, 0.54, 0.93, 0.51);
			}
			glm::mat4 T_players_bullet = glm::mat4(1.0f);
			NPlayers[i].t.x = GetPlayerX(i);
			NPlayers[i].t.z = GetPlayerZ(i);
			T_players_bullet = glm::translate(T_players_bullet, NPlayers[i].t);
			//std::cout << NPlayers[i].id << " | " << NPlayers[i].t.x << " | " << NPlayers[i].t.z << std::endl;
			all = T_players_bullet;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			GenShpere(NPlayers[i].ball, 0, 0.4, 20);
		}
		
	}
	int check_player_hp = 0;
	for (int i = 0; i < MAX_USER; ++i) {
		if (NPlayers[i].hp <= 0) {
			check_player_hp++;
		}
		else
		{
			check_player_hp = 0;
		}
	}
	if (check_player_hp == 3) {

		PostQuitMessage(0);
		send_dead_packet();
		NetCleanup();
	}
	{
		// 가짜 방들
		glFrontFace(GL_CW);
		glUniform3f(lightColorLocation, 0.2, 0.2, 0.2);
		glUniform3f(objColorLocation, 0.2, 0.2, 0.2);
		for (int i = 0; i < 4; i++)
		{
			T_field = glm::mat4(1.0f);
			T_field = translate(T_field, map.rooms_t[i]);
			all = T_field * S_rooms;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			glBindVertexArray(vao_map);
			glBindTexture(GL_TEXTURE_2D, Texture_Floor);
			glDrawArrays(GL_TRIANGLES, 24, 6);

			glBindTexture(GL_TEXTURE_2D, Texture_Wall[2]);
			glDrawArrays(GL_TRIANGLES, 30, 6);

			if (i != 2) glDrawArrays(GL_TRIANGLES, 0, 6);
			if (i != 3) glDrawArrays(GL_TRIANGLES, 6, 6);
			if (i != 1) glDrawArrays(GL_TRIANGLES, 12, 6);
			if (i != 0) glDrawArrays(GL_TRIANGLES, 18, 6);
		}
		glFrontFace(GL_CCW);
		// 총알
		glUniform3f(objColorLocation, 1.0, 1.0, 0.0);
		for (int i = 0; i < MAX_BULLET_NUM; i++)
		{	
			NetBullets[i].is_active = GetBulletState(i);
			if (NetBullets[i].is_active == true)
			{
				//std::cout << i << "|" << NetBullets[i].x << std::endl;
				glm::mat4 T_bullet = glm::mat4(1.0f);
				NetBullets[i].x = GetBulletX(i);
				NetBullets[i].y = GetBulletY(i);
				NetBullets[i].z = GetBulletZ(i);
				T_bullet = glm::translate(T_bullet, { NetBullets[i].x,NetBullets[i].y,NetBullets[i].z });
				all = T_bullet;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
				GenShpere(NetBullet_qobj, 1, 0.03, 6);
			}
		}
		// 적들
		for (int i = 0; i < 20; i++)
		{
			glUniform1f(FragKindLocation, 1.0f);
			//std::cout << std::boolalpha << enemy[i].in_room << std::endl;
			enemy[i].in_room = GetEnemyState(i);
			if (enemy[i].in_room)
			{
				glm::mat4 T_enemy = glm::mat4(1.0f);
				glm::mat4 S_enemy = glm::mat4(1.0f);
				glm::mat4 R_enemy = glm::mat4(1.0f);
				enemy[i].t.x = GetEnemyX(i);
				enemy[i].t.y = GetEnemyY(i);
				enemy[i].t.z = GetEnemyZ(i);
				//std::cout << i << " | " << enemy[i].t.x << " | " << enemy[i].t.y << " | " << enemy[i].t.z << std::endl;
				//std::cout << i << " | " << enemy[i].s.x << " | " << enemy[i].s.y << " | " << enemy[i]..z << std::endl;
				T_enemy = glm::translate(T_enemy, enemy[i].t);
				GLfloat tmp_s = GetEnemyS(i);
				enemy[i].s = { tmp_s ,tmp_s ,tmp_s };
				S_enemy = glm::scale(S_enemy, enemy[i].s);

				if (enemy[i].kind == 4)
				{
					R_enemy = glm::rotate(R_enemy, enemy[i].ry, { 0,1,0 });
					all = T_enemy * R_enemy  * S_enemy;
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
				}

				else
				{
					R_enemy = glm::rotate(R_enemy, enemy[i].ry, { 0,1,0 });
					all = T_enemy * R_enemy * S_enemy;
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
				}
				enemy[i].ry += 0.1f;
				glUniform3f(objColorLocation, 0.11, 0.33, 0.99);
				glBindVertexArray(vao_enemy);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

			glUniform1f(FragKindLocation, 1.0f);
			glUniform3f(objColorLocation, 1.0, 1.0, 1.0);
			glEnable(GL_BLEND);
			glBindTexture(GL_TEXTURE_2D, Texture_Particle);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (int j = 0; j < 3; j++)
			{
				if (enemy[i].particle_pop[j])
				{
					for (int k = 0; k < 30; k++)
					{
						glm::mat4 T_particle = glm::mat4(1.0f);
						glm::mat4 S_particle = glm::mat4(1.0f);
						glm::mat4 Rx_particle = glm::mat4(1.0f);
						glm::mat4 Ry_particle = glm::mat4(1.0f);

						T_particle = glm::translate(T_particle, enemy[i].particles[j][k].t);
						S_particle = glm::scale(S_particle, enemy[i].particles[j][k].s);
						Ry_particle = glm::rotate(Ry_particle, CalculateRotate({ 0,0,1.0f }, { enemy[i].particles[j][k].p_vector.x,0,enemy[i].particles[j][k].p_vector.z }, true), { 0,1,0 });
						all = T_particle * Ry_particle * S_particle;
						glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
						glBindVertexArray(vao_particle);
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
				}
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_BLEND);
		}
	}

	//크로스 헤어
	glUniform3f(ambientLightColorLocation, 1.0, 1.0, 1.0);

	glUniform1f(FragKindLocation, 0.0f);
	cameraPos = glm::vec3(0.0f, 0.0f, -0.01f); //--- 카메라 위치
	cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
	view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	projection = glm::mat4(1.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, 0.0));
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	all = glm::mat4(1.0f);
	all = glm::scale(all, glm::vec3(1, 1, 1));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
	glUniform3f(objColorLocation, 0.0, 1.0, 0.0);
	glBindVertexArray(vao_cross);
	glDrawArrays(GL_LINES, 0, 4);

	// ==================================플레이어 체력===============================
	GLUquadricObj* hp_qobj{};
	// 내 체력
	for (int i = 0; i < GetPlayerHP(myID); ++i) {
		all = glm::mat4(1.0f);
		all = glm::translate(all, glm::vec3(0.9f - 0.15f * float(i), -0.9f, 0));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
		glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);
		GenShpere(hp_qobj, 1, 0.07, 5);
	}
	// 팀원 체력
	int otherID[2] = {};

	if (myID == 0) {
		otherID[0] = 1;
		otherID[1] = 2;
	}
	else if (myID == 1) {
		otherID[0] = 0;
		otherID[1] = 2;
	}
	else if (myID == 2) {
		otherID[0] = 0;
		otherID[1] = 1;
	}
	for (int c = 0; c < GetClientNum() - 1; ++c) {

		if (otherID[c] == 0) {
			glUniform3f(objColorLocation, 0.95, 0.93, 0.4);
		}
		if (otherID[c] == 1) {
			glUniform3f(objColorLocation, 0.66, 0.25, 1.0);
		}
		if (otherID[c] == 2) {
			glUniform3f(objColorLocation, 0.54, 0.93, 0.51);
		}
		all = glm::mat4(1.0f);
		all = glm::translate(all, glm::vec3(0.9f, 0.7f - 0.12f * float(c), 0));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
		GenShpere(hp_qobj, 1, 0.05, 5);

		for (int i = 0; i < GetPlayerHP(otherID[c]); ++i) {
			all = glm::mat4(1.0f);
			all = glm::translate(all, glm::vec3(0.75f - 0.07f * float(i), 0.7f - 0.12f * float(c), 0));
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);
			GenShpere(hp_qobj, 1, 0.03, 5);
		}
	}

	for (int i = 0; i < GetPlayerHP(myID); ++i) {
		all = glm::mat4(1.0f);
		all = glm::translate(all, glm::vec3(0.9f - 0.15f * float(i), -0.9f, 0));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
		glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);
		GenShpere(hp_qobj, 1, 0.07, 5);
	}
	//-----------------------//
	glViewport(width - width / 5 - 10, height - height / 5 - 10, height / 5, height / 5);
	glUniform1f(FragKindLocation, 0.0f);
	cameraPos = glm::vec3(0.0f, 50.0f, 0.0f);
	cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 0.0f, -1.0f);
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	projection = glm::ortho(-4.0, 4.0, -4.0, 4.0, -4.0, 60.0);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	//중간 방 바닥,천장
	T_field = glm::mat4(1.0f);
	T_field = translate(T_field, map.floor_t);
	all = T_field * S_field;
	glUniform3f(objColorLocation, 1.0, 1.0, 1.0);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
	glBindVertexArray(vao_enemy);
	glDrawArrays(GL_TRIANGLES, 30, 6);

	glUniform3f(objColorLocation, 1.0, 1.0, 0.0);
	for (int i = 0; i < 3; ++i) {
		if (NPlayers[i].hp < 1) continue;
		if (i == 0) {
			glUniform3f(objColorLocation, 0.95, 0.93, 0.4);
		}
		if (i == 1) {
			glUniform3f(objColorLocation, 0.66, 0.25, 1.0);
		}
		if (i == 2) {
			glUniform3f(objColorLocation, 0.54, 0.93, 0.51);
		}
		if (NPlayers[i].active) {
			glm::mat4 T_players = glm::mat4(1.0f);
			NPlayers[i].t.x = GetPlayerX(i);
			NPlayers[i].t.z = GetPlayerZ(i);
			T_players = glm::translate(T_players, NPlayers[i].t);
			all = T_players;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			GenShpere(NPlayers[i].ball, 0, 0.4, 5);
		}

		else {
			glm::mat4 T_players = glm::mat4(1.0f);
			T_players = glm::translate(T_players, player.t);
			all = T_players;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			GenShpere(NPlayers[i].ball, 0, 0.4, 5);
		}
	}

	for (int i = 0; i < MAX_ENEMY_NUM; i++)
	{
		glUniform3f(objColorLocation, 0.11, 0.33, 0.99);
		if (enemy[i].in_room == true)
		{
			glm::mat4 T_enemy = glm::mat4(1.0f);
			glm::mat4 S_enemy = glm::mat4(1.0f);

			T_enemy = glm::translate(T_enemy, enemy[i].t);
			S_enemy = glm::scale(S_enemy, enemy[i].s);

			all = T_enemy * S_enemy;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(all));
			glBindVertexArray(vao_enemy);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	//------------------------- 조명
	if (GetisHit(myID))
	{
		map.LightColor[1] = 0.0f;
		map.LightColor[2] = 0.0f;
	}
	else
	{
		map.LightColor[1] = 1.0f;
		map.LightColor[2] = 1.0f;
	}

	glutSwapBuffers();
}

GLvoid TimerFunction(int value)
{
	glutSetCursor(GLUT_CURSOR_NONE); // 마우스 커서 없애기

	view_control.Direction = { cos(3.141592 * view_control.LR / 180.0f), 	// 보는 방향 바꾸기
		view_control.UD, sin(3.141592 * view_control.LR / 180.0f) };
	// 적 이동방향 계산 및 이동

	player.t.x = GetPlayerX(myID);
	player.t.z = GetPlayerZ(myID);
	//Timer_PlayerRun();
	Timer_Enemy_Actions();
	int stage = GetStageState();
	if (stage == -1) {
		for (int i = 0; i < MAX_ENEMY_NUM; ++i)
			for (int j = 0; j < 3; ++j)
				for (int q = 0; q < 30; q++)
				{
					float speed = (float)abs(dist(gen)) / 1000.0f;
					enemy[i].particles[j][q].t = { 0,0,0 };
					enemy[i].particles[j][q].p_vector = { 0,0,0 };
					enemy[i].particles[j][q].time = 0;
					enemy[i].particles[j][q].bounce_time = 0;
					enemy[i].particles[j][q].vector = glm::normalize(glm::vec3(dist(gen), abs(dist(gen)) + 200, dist(gen)));
					enemy[i].particles[j][q].vector = speed * enemy[i].particles[j][q].vector;
					enemy[i].particles[j][q].s.x = (float)abs(dist(gen)) / 2000.0f + 0.05f;
					enemy[i].particles[j][q].s.y = enemy[i].particles[j][q].s.x;
					enemy[i].particles[j][q].s.z = enemy[i].particles[j][q].s.x;
				}
	}

	clock_t end = clock();
	double time = double(end - start) / CLOCKS_PER_SEC;
	if (time > 0.01f) {
		start = clock();
		send_direction_packet(view_control.Direction.x, view_control.Direction.z, view_control.LR);
	}
	glutPostRedisplay();

	//std::cout << player.LR_dir << " "<< player.FB_dir << "\n";
	if (timefunc_flag == 1)
	{
		glutTimerFunc(20, TimerFunction, 1);
	}
}

void Timer_Enemy_Actions()
{
	for (int i = 0; i < MAX_ENEMY_NUM; i++)// 적 움직임
	{
		//파티클
		for (int j = 0; j < 3; j++)
		{	
			enemy[i].particle_pop[j] = GetEnemyPopState(i, j);
			if (enemy[i].particle_pop[j])
			{
				for (int k = 0; k < 30; k++)
				{
					if (enemy[i].particles[j][k].time < 0.01f) {
						enemy[i].particles[j][k].t = enemy[i].t;
					}
					enemy[i].particles[j][k].p_vector = player.t - enemy[i].particles[j][k].t;
					enemy[i].particles[j][k].t += glm::vec3(enemy[i].particles[j][k].vector.x,
						enemy[i].particles[j][k].vector.y - 0.005 * enemy[i].particles[j][k].time,
						enemy[i].particles[j][k].vector.z);
					if (enemy[i].particles[j][k].t.y < -0.4 && enemy[i].particles[j][k].bounce_time < 4)
					{
						enemy[i].particles[j][k].time = enemy[i].particles[j][k].time * 0.5f;
						enemy[i].particles[j][k].bounce_time += 1;
					}

					if (enemy[i].particles[j][0].time > 100)
					{
						enemy[i].particle_pop[j] = false;
					}

					for (int a = 0; a < 4; a++)
					{
						for (int b = 0; b < 10; b++)
						{
							if (collide_box(enemy[i].particles[j][k].t, map.pillar_t[a][b], enemy[i].particles[j][k].s, { 1.0f,6.0f,1.0f }))
							{
								enemy[i].particles[j][k].t -= glm::vec3(enemy[i].particles[j][k].vector.x,
									enemy[i].particles[j][k].vector.y - 0.03 * enemy[i].particles[j][k].time,
									enemy[i].particles[j][k].vector.z);
								enemy[i].particles[j][k].vector = CC_CalculateRVector(enemy[i].particles[j][k].vector, map.pillar_normal[a]);
								break;
							}
						}
					}
					enemy[i].particles[j][k].time += 1.0f;

				}
			}
		}
	}
}

glm::vec3 CC_CalculateRVector(glm::vec3 input, glm::vec3 normal)
{
	return input - 2.0f * (glm::dot(normal, input)) * normal;
}
       
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'W':
		if (player.FB_dir <= 0) {
			player.FB_dir += 1;
			send_keyboard_packet(1);
		}
		break;
	case 'A':
		if (player.LR_dir >= 0) {
			player.LR_dir -= 1;
			send_keyboard_packet(2);
		}
		break;
	case 'S':
		if (player.FB_dir >= 0) {
			player.FB_dir -= 1;
			send_keyboard_packet(3);
		}

		break;
	case 'D':
		if (player.LR_dir <= 0) {
			player.LR_dir += 1;
			send_keyboard_packet(4);
		}
		break;
	case 'Q':
		PostQuitMessage(0);
		break;
	case 'w':
		if (player.FB_dir <= 0) {
			player.FB_dir += 1;
			send_keyboard_packet(1);
		}
		break;
	case 'a':
		if (player.LR_dir >= 0) {
			player.LR_dir -= 1;
			send_keyboard_packet(2);
		}
		break;
	case 's':
		if (player.FB_dir >= 0) {
			player.FB_dir -= 1;
			send_keyboard_packet(3);
		}

		break;
	case 'd':
		if (player.LR_dir <= 0) {
			player.LR_dir += 1;
			send_keyboard_packet(4);
		}
		break;
	case 'q':
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	//glutPostRedisplay();
}

GLvoid UpKeyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'W':
		send_keyboard_packet(-1);
		player.FB_dir -= 1;
		break;
	case 'A':
		send_keyboard_packet(-2);
		player.LR_dir += 1;
		break;
	case 'S':
		send_keyboard_packet(-3);
		player.FB_dir += 1;
		break;
	case 'D':
		send_keyboard_packet(-4);
		player.LR_dir -= 1;
		break;
	case 'w':
		send_keyboard_packet(-1);
		player.FB_dir -= 1;
		break;
	case 'a':
		send_keyboard_packet(-2);
		player.LR_dir += 1;
		break;
	case 's':
		send_keyboard_packet(-3);
		player.FB_dir += 1;
		break;
	case 'd':
		send_keyboard_packet(-4);
		player.LR_dir -= 1;
		break;
	default:
		break;
	}
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (NPlayers[myID].hp > 0) {
		if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
		{
			glm::vec3 tmp = glm::normalize(view_control.Direction);
			send_attack_packet(tmp.x, tmp.y, tmp.z);

			glutPostRedisplay();
		}
	}

	//if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	//{
	//	std::cout << "x = " << x << "y = " << y << std::endl;
	//	std::cout << "X = " << X << "Y = " << Y << std::endl;
	//}
}

GLvoid MouseEntry(int state)
{
	if (state == GLUT_LEFT)
	{
		glutWarpPointer(width / 2, height / 2);
		view_control.previous_x = width / 2; view_control.previous_y = height / 2;
		//glutWarpPointer(width / 2, height / 2);
		//view_control.previous_x = width / 2; view_control.previous_y = height / 2;
	}
}

GLvoid Motion(int x, int y)
{
	CalculateMouseDelta(x, y);
}

GLvoid PassiveMotion(int x, int y)
{
	CalculateMouseDelta(x, y);
}

bool collide_sphere(glm::vec3 a, glm::vec3 b, float coll_dist)
{
	glm::vec3 c = a - b;

	float dist = sqrt(c.x * c.x + c.y * c.y + c.z * c.z);

	if (coll_dist > dist)
	{
		return true;
	}

	return false;
}

bool collide_box(glm::vec3 bb, glm::vec3 tb, glm::vec3 bb_scale, glm::vec3 tb_scale)
{
	float size = 0.5;

	glm::vec3 bb_min = { bb.x - size * bb_scale.x,bb.y - size * bb_scale.y,bb.z - size * bb_scale.z };
	glm::vec3 bb_max = { bb.x + size * bb_scale.x,bb.y + size * bb_scale.y,bb.z + size * bb_scale.z };

	glm::vec3 tb_min = { tb.x - size * tb_scale.x ,tb.y - size * tb_scale.y,tb.z - size * tb_scale.z };
	glm::vec3 tb_max = { tb.x + size * tb_scale.x,tb.y + size * tb_scale.y,tb.z + size * tb_scale.z };

	if (bb_min.x <= tb_max.x &&
		bb_max.x >= tb_min.x &&
		bb_min.y <= tb_max.y &&
		bb_max.y >= tb_min.y &&
		bb_min.z <= tb_max.z &&
		bb_max.z >= tb_min.z)
	{
		return true;
	}

	return false;
}

float CalculateRotate(glm::vec3 n, glm::vec3 t, bool normal_z)
{
	float dot = glm::dot(n, t);
	float det = (n.x * n.x + n.y * n.y + n.z * n.z) * (t.x * t.x + t.y * t.y + t.z * t.z);

	float a = acos(dot / sqrt(det));

	//std::cout << a << " "<< dot / sqrt(det) <<"\n";

	//if (abs(t.y) > 0) return -a;
	if (normal_z == true && t.x < 0) return -a;

	return a;
}

void CalculateMouseDelta(int x, int y)
{
	float tmp[2];
	tmp[0] = x - view_control.previous_x;
	tmp[1] = y - view_control.previous_y;
	view_control.dpi = 10.0f;

	if ((float)tmp[1] / 100.0f < 1.0f)
	{
		view_control.UD -= float(tmp[1]) / 20.0f / view_control.dpi;
	}

	if (float(tmp[0] / view_control.dpi) / 100.0f < 10.0f)
	{
		view_control.LR += float(tmp[0] / view_control.dpi);
	}

	view_control.previous_x = x; view_control.previous_y = y;
}

GLvoid GenShpere(GLUquadricObj* obj, bool shpere, float r, int detail)
{
	obj = gluNewQuadric();

	gluQuadricDrawStyle(obj, GLU_FILL);
	gluQuadricNormals(obj, GLU_SMOOTH);
	gluQuadricOrientation(obj, GLU_OUTSIDE);

	gluSphere(obj, r, detail, detail);
}

void InitBuffer()
{
	// 바닥, 벽
	glGenVertexArrays(1, &vao_map);
	glGenBuffers(3, vbo_map);

	glBindVertexArray(vao_map);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[0]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[1]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa_normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[2]);
	glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), Hexa_texel, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	// 적
	glGenVertexArrays(1, &vao_enemy);
	glGenBuffers(2, vbo_enemy);

	glBindVertexArray(vao_enemy);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_enemy[0]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_enemy[1]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa_normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// 파티클
	glGenVertexArrays(1, &vao_particle);
	glGenBuffers(2, vbo_particle);

	glBindVertexArray(vao_particle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_particle[0]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), Rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_particle[1]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), Rect_texel, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	// 바닥, 벽
	glGenVertexArrays(1, &vao_map);
	glGenBuffers(3, vbo_map);

	glBindVertexArray(vao_map);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[0]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[1]);
	glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), Hexa_normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_map[2]);
	glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), Hexa_texel, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// 크로스 헤어
	glGenVertexArrays(1, &vao_cross);
	glGenBuffers(1, &vbo_cross);
	glBindVertexArray(vao_cross);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cross);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), crosshair, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void InitTexture()
{
	int Texture_Wall_Size_1 = 800;
	int Texture_Wall_Size_2 = 1000;
	int Texture_Wall_Size_3 = 1024;
	int Texture_Floor_Size = 1024;
	int Texture_Particle_Size = 100;
	int numberOfChannel;

	glUseProgram(s_program);
	//================================================= ▼ 벽 텍스쳐 2
	glBindTexture(GL_TEXTURE_2D, Texture_Wall[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data3 = stbi_load("Wall_6.png", &Texture_Wall_Size_3, &Texture_Wall_Size_3, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Texture_Wall_Size_3, Texture_Wall_Size_3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data3);
	stbi_image_free(data3);
	//================================================= ▼ 바닥 텍스쳐
	glGenTextures(1, &Texture_Floor);
	glBindTexture(GL_TEXTURE_2D, Texture_Floor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data4 = stbi_load("Floor 3.png", &Texture_Floor_Size, &Texture_Floor_Size, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Texture_Floor_Size, Texture_Floor_Size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data4);
	stbi_image_free(data4);
	//================================================= ▼ 파티클 텍스쳐
	glGenTextures(1, &Texture_Particle);
	glBindTexture(GL_TEXTURE_2D, Texture_Particle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data5 = stbi_load("particle.png", &Texture_Particle_Size, &Texture_Particle_Size, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Texture_Particle_Size, Texture_Particle_Size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data5);
	stbi_image_free(data5);
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기asd
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);
	glLinkProgram(s_program);

	//checkCompileErrors(s_program, "PROGRAM");

	//--- 세이더 삭제하기
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	//--- Shader Program 사용하기
	glUseProgram(s_program);
}

bool make_vertexShader()
{
	char a[20] = "vertex.glsl";

	vertexsource = filetobuf(a);
	//--- 버텍스 세이더 객체 만들기
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexshader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog;
		return false;
	}

	return true;
}

bool make_fragmentShader()
{
	char a[20] = "fragment.glsl";;

	fragmentsource = filetobuf(a);
	//--- 프래그먼트 세이더 객체 만들기
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentshader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog;
		return false;
	}

	return true;
}

char* filetobuf(char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fopen_s(&fptr, file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer 
}

void RecvEnemyInfo () {
	for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
		enemy[i].in_room = true;
		enemy[i].shot = false;
		enemy[i].hp = GetEnemyHP(i);
		enemy[i].kind = GetEnemyKind(i);
		enemy[i].t.x = GetEnemyX(i);
		enemy[i].t.y = GetEnemyY(i);
		enemy[i].t.z = GetEnemyZ(i);

		if (enemy[i].kind == 4) {
			enemy[i].s.x = 0.4f;
			enemy[i].s.y = 0.4f;
			enemy[i].s.z = 0.4f;
		}
		else if (enemy[i].kind < 4){
			enemy[i].s.x = (float)enemy[i].hp * 0.25f;
			enemy[i].s.y = (float)enemy[i].hp * 0.25f;
			enemy[i].s.z = (float)enemy[i].hp * 0.25f;
		}
		//std::cout << enemy[i].kind << std::endl;
		//std::cout << enemy[i].s.x << std::endl;

		for (int j = 0; j < 3; j++)
		{
			enemy[i].particle_pop[j] = false;
			for (int k = 0; k < 30; k++)
			{
				float speed = (float)abs(dist(gen)) / 1000.0f;
				enemy[i].particles[j][k].t = { 0,0,0 };
				enemy[i].particles[j][k].p_vector = { 0,0,0 };
				enemy[i].particles[j][k].time = 0;
				enemy[i].particles[j][k].bounce_time = 0;
				enemy[i].particles[j][k].vector = glm::normalize(glm::vec3(dist(gen), abs(dist(gen)) + 200, dist(gen)));
				enemy[i].particles[j][k].vector = speed * enemy[i].particles[j][k].vector;
				enemy[i].particles[j][k].s.x = (float)abs(dist(gen)) / 2000.0f + 0.05f;
				enemy[i].particles[j][k].s.y = enemy[i].particles[j][k].s.x;
				enemy[i].particles[j][k].s.z = enemy[i].particles[j][k].s.x;
			}
		}
	}
}

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	NetInit();
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)do_recv, (LPVOID)NULL, 0, NULL);

	while (!GetGameState());
	RecvEnemyInfo();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	width = height = 900;
	glutInitWindowSize(width, height);
	glutCreateWindow("Perfect Shape");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;

	}
	else {
		std::cerr << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	}

	InitVariable(); //  초기화
	InitShader();
	InitBuffer();
	InitTexture();

	timefunc_flag = 1;
	for (int i = 0; i < 3; i++)
	{
		map.LightColor[i] = 1.0f;
	}

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(UpKeyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutPassiveMotionFunc(PassiveMotion);
	glutEntryFunc(MouseEntry);
	glutTimerFunc(20, TimerFunction, 1);
	glutMainLoop();

	NetCleanup();
}