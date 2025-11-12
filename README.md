# Webserv

C++98로 구현한 HTTP/1.1 웹 서버 프로젝트입니다. Nginx와 유사한 설정 방식을 사용하며, 정적 파일 제공, CGI 실행, 파일 업로드 등 다양한 기능을 제공합니다.

## 주요 기능

- **HTTP/1.1 프로토콜 지원**: 표준 HTTP 요청/응답 처리
- **멀티 서버 지원**: 여러 포트에서 동시에 가상 호스트 운영
- **이벤트 드리븐 아키텍처**: kqueue를 사용한 비동기 I/O 처리
- **HTTP 메소드 지원**: GET, POST, PUT, DELETE, HEAD
- **CGI 지원**: 동적 콘텐츠 생성을 위한 CGI 스크립트 실행
- **파일 업로드**: POST/PUT 메소드를 통한 파일 업로드
- **리다이렉션**: 301 리다이렉트 지원
- **자동 인덱싱**: 디렉토리 목록 자동 생성
- **커스텀 에러 페이지**: HTTP 상태 코드별 맞춤 에러 페이지
- **요청 바디 크기 제한**: Location별 업로드 크기 제한 설정

## 프로젝트 구조

```
webserv/
├── config/
│   └── default.conf           # 기본 서버 설정 파일
├── html/
│   ├── error_pages/           # 에러 페이지 템플릿
│   ├── www/                   # 기본 웹사이트 파일
│   └── YoupiBanane/           # 테스트 콘텐츠
├── srcs/
│   ├── Client.cpp/.hpp        # 클라이언트 연결 처리
│   ├── ClientManager.cpp/.hpp # 다중 클라이언트 관리
│   ├── Server.cpp/.hpp        # 서버 설정 클래스
│   ├── ServerManager.cpp/.hpp # 멀티 서버 관리
│   ├── Event.cpp/.hpp         # Kqueue 이벤트 처리
│   ├── Location.cpp/.hpp      # Location 블록 처리
│   ├── Config/
│   │   ├── ConfigParser.cpp/.hpp      # 설정 파일 파싱
│   │   └── ConfigFunctions.cpp        # 설정 헬퍼 함수
│   ├── Message/
│   │   ├── Message.cpp/.hpp   # HTTP 메시지 기본 클래스
│   │   ├── Request.cpp/.hpp   # HTTP 요청 파싱
│   │   └── Response.cpp/.hpp  # HTTP 응답 생성
│   ├── Http/
│   │   ├── HttpRequestManager.cpp/.hpp  # 요청 처리 오케스트레이션
│   │   └── Handler/
│   │       ├── Handler.cpp/.hpp         # 핸들러 기본 클래스
│   │       ├── StaticHandler.cpp/.hpp   # 정적 파일 서빙
│   │       ├── DynamicHandler.cpp/.hpp  # CGI 처리
│   │       ├── ErrorHandler.cpp/.hpp    # 에러 응답
│   │       ├── DeleteHandler.cpp/.hpp   # DELETE 요청 처리
│   │       ├── RedirectHandler.cpp/.hpp # 리다이렉트 처리
│   │       └── HttpStatusCodes.cpp/.hpp # HTTP 상태 코드
│   └── main.cpp               # 프로그램 진입점
├── tester/                    # 테스트 스크립트
├── Makefile                   # 빌드 설정
└── README.md
```

## 요구사항

- **컴파일러**: C++98 표준을 지원하는 C++ 컴파일러
- **운영체제**: BSD 계열 (macOS, FreeBSD 등) - kqueue 사용
- **빌드 도구**: GNU Make

## 빌드

```bash
# 프로젝트 빌드
make

# 클린 빌드
make re

# 오브젝트 파일 삭제
make clean

# 전체 삭제 (실행 파일 포함)
make fclean
```

빌드가 완료되면 `webserv` 실행 파일이 생성됩니다.

## 사용법

### 기본 실행

```bash
# 기본 설정 파일(config/default.conf) 사용
./webserv

# 커스텀 설정 파일 사용
./webserv path/to/config.conf
```

### 서버 테스트

```bash
# 웹 브라우저에서 접속
http://localhost:80

# curl을 사용한 테스트
curl http://localhost:80
curl -X POST -d "data=test" http://localhost:80/post_test
curl -X DELETE http://localhost:80/soulee/test.html
```

## 설정 파일 구조

설정 파일은 Nginx와 유사한 문법을 사용합니다.

### 기본 서버 블록

```nginx
server {
    listen 80;                      # 리스닝 포트
    server_name localhost;          # 서버 이름
    error_page 403 404 405 40x.html;  # 에러 페이지
    upload_path uploaded_files;     # 업로드 디렉토리

    location / {
        allow_method GET;           # 허용 HTTP 메소드
        root ./html/www;            # 문서 루트
        index index.html index.htm; # 기본 인덱스 파일
        autoindex on;               # 자동 인덱싱 활성화
    }
}
```

### Location 블록 설정

```nginx
location /api {
    allow_method GET POST DELETE;   # 다중 메소드 허용
    root ./html/api;
    index index.html;
    autoindex on;
}

location /upload {
    allow_method POST PUT;
    root ./html/uploads;
    client_max_body_size 1000;      # 최대 바디 크기 (바이트)
}

location /redirect {
    allow_method GET;
    return 301 http://www.example.com;  # 리다이렉트
}
```

### 설정 지시어 설명

| 지시어 | 설명 | 예시 |
|--------|------|------|
| `listen` | 리스닝 포트 번호 | `listen 80;` |
| `server_name` | 가상 호스트 이름 | `server_name localhost;` |
| `error_page` | 커스텀 에러 페이지 | `error_page 404 error.html;` |
| `upload_path` | 파일 업로드 경로 | `upload_path uploads;` |
| `allow_method` | 허용할 HTTP 메소드 | `allow_method GET POST;` |
| `root` | 문서 루트 디렉토리 | `root ./html/www;` |
| `index` | 기본 인덱스 파일 | `index index.html;` |
| `autoindex` | 자동 디렉토리 인덱싱 | `autoindex on;` |
| `client_max_body_size` | 최대 요청 바디 크기 | `client_max_body_size 1000;` |
| `return` | HTTP 리다이렉트 | `return 301 http://...;` |

## HTTP 메소드 지원

### GET
정적 파일을 제공하거나 CGI 스크립트를 실행합니다.

```bash
curl http://localhost:80/index.html
```

### POST
데이터를 서버로 전송하거나 파일을 업로드합니다.

```bash
curl -X POST -d "key=value" http://localhost:80/post_body
curl -X POST -F "file=@test.txt" http://localhost:80/upload
```

### PUT
파일을 생성하거나 업데이트합니다.

```bash
curl -X PUT -d "content" http://localhost:80/put_test/file.txt
```

### DELETE
지정된 리소스를 삭제합니다.

```bash
curl -X DELETE http://localhost:80/soulee/file.txt
```

## CGI 지원

웹서버는 동적 콘텐츠 생성을 위해 CGI(Common Gateway Interface)를 지원합니다.

### CGI 스크립트 예시

```python
#!/usr/bin/env python3
print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("</body></html>")
```

CGI 스크립트는 실행 권한이 필요하며, 적절한 shebang 라인을 포함해야 합니다.

## 아키텍처

### 요청 처리 흐름

```
Client Request
    ↓
ServerManager (kqueue 이벤트 대기)
    ↓
ClientManager (클라이언트 연결 관리)
    ↓
Request Parser (HTTP 요청 파싱)
    ↓
HttpRequestManager (요청 라우팅)
    ↓
Handler 선택 (Static/Dynamic/Error/Redirect/Delete)
    ↓
Response Builder (HTTP 응답 생성)
    ↓
Client Response
```

### 핵심 컴포넌트

- **ServerManager**: 여러 서버 인스턴스를 관리하고 kqueue를 통해 이벤트를 처리
- **ClientManager**: 클라이언트 연결을 추적하고 관리
- **ConfigParser**: 설정 파일을 파싱하여 서버 구성 생성
- **HttpRequestManager**: HTTP 요청을 적절한 핸들러로 라우팅
- **Handler 계층**: 요청 유형별로 특화된 처리 로직 제공

## 기술 스택

- **언어**: C++98
- **표준 라이브러리**: STL (Standard Template Library)
- **I/O 모델**: kqueue (BSD 이벤트 알림 인터페이스)
- **프로토콜**: HTTP/1.1
- **빌드 시스템**: GNU Make
- **컴파일 플래그**: `-Wall -Werror -Wextra -std=c++98 -g3`

## 개발 히스토리

프로젝트는 활발히 개발 중이며, 최근 커밋 내역은 다음과 같습니다:

- CGI 에러 처리 개선
- 파일 존재하지 않을 때 500 에러 반환
- 서버 빌드 최적화

## 테스트

`tester/` 디렉토리에 테스트 스크립트가 포함되어 있습니다. 서버의 다양한 기능을 검증하는 데 사용할 수 있습니다.

## 라이센스

이 프로젝트는 교육 목적으로 제작되었습니다.

## 기여자

Git 히스토리를 참조하여 프로젝트 기여자를 확인할 수 있습니다.

---

더 많은 정보나 버그 리포트는 프로젝트 저장소의 이슈 트래커를 이용해주세요.
