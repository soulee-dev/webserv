# webserv


### Kq.cpp에 ServeStatic을 제외한 대다수의 기능들이 포함되어 있는데 파일들을 분리해야함

### config file 파싱, 서버 블록 여러 개 들어올 때 서버가 여러 개 열리도록 처리해야 함 -> 진행중

### 현재 Makefile에 포함된 main.cpp Kq.cpp ServerManager.cpp Static.cpp 을 제외한 부분은 Config 파일을 파싱하기 위한 코드이며 Jaemjeon님의 코드를 참고하고 있음

### kqueue를 사용하여 복수의 클라이언트를 처리할 수 있어야 함 -> 가능해짐 
