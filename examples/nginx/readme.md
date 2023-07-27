# NGINX 실습 하기
- 서버 별 포트를 다르게 설정해봅니다
- `server_name`을 다르게 설정해봅니다
- default 에러 페이지를 설정해봅니다
- `client_max_body_size`를 다르게 설정해봅니다
	- Insomnia를 사용해 파일을 전송해 봅니다
- 여러 웹 경로를 만들어 봅니다
	- 경로별 root를 다르게 지정해 봅니다
- 디렉토리 리스팅 기능을 사용해 봅니다
- `index` 옵션을 변경해 봅니다
- 파일 저장 경로를 지정해 봅니다
- Basic Auth를 설정해 봅니다
- 같은 포트에 두 개의 서버를 열고, 어떤 서버 블록이 실행되는지 확인해 봅니다
- 301을 리턴하여 다른 웹사이트로 redirection 해봅니다

# 도커 실행 명령어
```sh
docker compose up --build
```