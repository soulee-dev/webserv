#include "../includes/Server.hpp"
#include "../includes/color.hpp"

std::string getFileType(std::string file_name);

void Server::ServeStatic(Request &req)
{
    std::string file_type = getFileType(req.file_name);
    ssize_t sent_bytes;
    std::vector<char> buffer;
    std::string header;
    std::ifstream file(req.file_name, std::ios::binary);

    // get length of file:
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);
    buffer.resize(length);
    if (!file.read(&buffer[0], length))
    {
        throw std::runtime_error("Failed to read file");
    }

    header = BuildHeader("200 OK", length, file_type, cookies);
    std::vector<char> response(header.begin(), header.end());
    response.insert(response.end(), buffer.begin(), buffer.end());

    // Set the socket to Non-blocking mode
    int flags = fcntl(req.fd, F_GETFL, 0);
    fcntl(req.fd, F_SETFL, flags | O_NONBLOCK);

    ssize_t sent_bytes_total = 0;
    ssize_t bytes_to_send = response.size();

    while (sent_bytes_total < response.size())
    {
        ssize_t bytes_sent = send(req.fd, &response[sent_bytes_total], bytes_to_send, 0);

        if (bytes_sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 리소스가 일시적으로 부족한 경우, 잠시 기다렸다가 다시 시도
                continue;
            }
            else
            {
                // 다른 오류가 발생한 경우, 에러 처리
                throw std::runtime_error("Send failed");
            }
        }
        else if (bytes_sent == 0)
        {
            // 클라이언트 연결이 끊어진 경우
            break;
        }

        sent_bytes_total += bytes_sent;
        bytes_to_send -= bytes_sent;
    }

    std::cout << "sent_bytes : " << sent_bytes_total << '\n';
    std::cout << "response size : " << response.size() << '\n';

    if (sent_bytes_total == static_cast<ssize_t>(response.size()))
        std::cout << "Successfully send message" << std::endl;
    else
        std::cout << "Serve Static Error" << std::endl;
}

std::string getFileType(std::string file_name)
{
    std::string file_type;

    if (file_name.find(".html") != std::string::npos || file_name.find(".htm") != std::string::npos)
        file_type = "text/html";
    else if (file_name.find(".gif") != std::string::npos)
        file_type = "image/gif";
    else if (file_name.find(".png") != std::string::npos)
        file_type = "image/png";
    else if (file_name.find(".jpg") != std::string::npos)
        file_type = "image/jpeg";
    else if (file_name.find(".mpg") != std::string::npos)
        file_type = "video/mpg";
    else if (file_name.find(".mp4") != std::string::npos)
        file_type = "video/mp4";
    else
        file_type = "text/plain";
    return (file_type);
}
