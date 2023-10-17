# NGP_project

# 개발 환경
-	OS : Windows 10
-	IDE : Visual Studio 2019
-	API : Win32 API / Windows Socket API
-	네트워크 IO모델 : 다중 쓰레드 모델
-	언어 : C / C++ (Open GL)


# 게임 개요
- 게임제목:  Perfect Shape
- 게임컨셉: 플레이어3명에서 캐릭터를 조종하여 적을 쓰러트리며 다음 방으로 진행하며 지속적으로 살아남는 1인칭 슈팅게임

<img src="https://github.com/doyoung930/NGP_project/assets/70666642/3bcc54b7-7b3e-41ae-91ea-b4ca7dca51a5" width="200" height="400"/>
# 게임플레이
- WSAD(상하좌우) 키보드를 통해 입력을 받아 플레이어를 움직일 수 있음.
- 마우스 왼쪽 커서를 통해 적을 공격
- 적에게 충돌하거나 적의 총알에 충돌할 경우 데미지를 입음.
- 게임의 흐름
	1. 스테이지 시작
	2. 모든 적 처치
	3. 다음 방 이동 (다음 스테이지 시작)

# Level Design

-high level Design
<img src="https://github.com/doyoung930/NGP_project/assets/70666642/b51f4c99-c58c-442e-a0ee-b22ff5f4fc08" width="200" height="400"/>

-Login Flow chart
<img src="https://github.com/doyoung930/NGP_project/assets/70666642/bbf03357-1415-49dc-beb6-41038447f4a9" width="200" height="400"/>

-Game Flow chart
<img src="https://github.com/doyoung930/NGP_project/assets/70666642/042d9090-94fb-4fcd-92bf-deafbe963436" width="200" height="400"/>
# 개인별 역할분담
1. 이도영
-	일정 조율 및 문서 작성
-	void send_login_packet() : 클라이언트가 접속하면 접속확인과 id를 보내는 함수
-	void send_add_packet() : 다른 클라이언트의 접속 전송 함수
-	void send_remove_packet() : 죽은 플레이어 삭제 함수
-	void Disconnect() : 플레이어 연결 종료 함수
-	Player_Dead_state() : 플레이어가 사망시 렌더링을 종료 하는 함수


2. 유재우
-	게임로직 수정
-	각종 렌더링 추가
-	void send_move_packet() : (클라이언트) 키 입력(W, A, S, D) 송신 함수
-	void send_attack_packet() : 마우스 좌(attack) 클릭 송신 함수
-	void send_move_packet() : (서버)클라이언트의 움직임 전송 함수
-	void send_bullet_packet() : 총알의 위치 값 전송 함수
-	int NetInit(), NetCleanup() : 서버 접속 함수
-	do_recv() : (클라)서버가 전송한 패킷 수신 함수 



3. 이도
-	bool player_collide() : 플레이어 충돌 판단 함수	
-	bool enemy_collide() : 적 충돌 판단 함수
-	DWORD WINAPI Send_all(): (서버)데이터 송신 함수.
-	void process_packet(); 패킷 재 조립 함수
-	void GameStart() : 게임이 시작 및 정보 초기화 함수
-	벽 충돌 체크 및 충돌 체크에 관련된 함수 및 버그 오류 수정
	( 적 충돌시 조명 변화, 충돌시 무적시간 추가, 벽 적 충돌체크간 오류 수정)

