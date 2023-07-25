#include "config.hpp"
#include "Location.hpp"
#include "Server.hpp"

bool Server::fillServer(std::map<std::string, Location> &mapLocations, std::map<std::string, std::string> &mapSentence)
{
	std::map<std::string, std::string>::iterator it = mapSentence.begin();

	this->locations = mapLocations; // 적어도 하나이상의 locationBlock이 있어야 한다.
	while (it != mapSentence.end())
	{
		if (it->first == "listen")
			this->setListen(it->second);
		else if (it->first == "server_name")
			this->setServerName(it->second);
		else if (it->first == "root")
			this->setRoot(it->second);
		else if (it->first == "error_page")
		{
			if (this->setErrorPage(it->second))
				return true;
		}
		else if (it->first == "return")
		{
			if (this->setRedirection(it->second))
				return true;
		}
		else if (it->first == "autoindex")
			this->setAutoIndex(it->second);
		else if (it->first == "client_max_body_size")
			this->setClientBodySize(it->second);
		else
			return true;
		it++;
	}
	return false;
}

bool Location::fillLocationBlock(std::map<std::string, std::string> &mapSentence)
{
	std::map<std::string, std::string>::iterator it = mapSentence.begin();

	while (it != mapSentence.end())
	{
		if (it->first == "root")
			this->setRoot(it->second);
		else if (it->first == "error_page")
		{
			if (this->setErrorPage(it->second))
				return true;
		}
		else if (it->first == "return")
		{
			if (this->setRedirection(it->second))
				return true;
		}
		else if (it->first == "allow_method")
		{
			if (this->setAllowMethod(it->second))
				return true;
		}
		else if (it->first == "autoindex")
			this->setAutoIndex(it->second);
		else if (it->first == "client_max_body_size")
			this->setClientBodySize(it->second);
		else if (it->first == "index")
			this->setIndex(it->second);
		else
			return true;
		it++;
	}
	return false;
}
void Server::setListen(std::string &input)
{
	long parsedPort = strtod(input.c_str(), NULL);

	if (parsedPort < 0 || parsedPort > 65535)
		std::cout << "Warning : invalid port number" << std::endl;
	this->listen = parsedPort;
}

void Server::setServerName(std::string &input)
{
	this->serverName = input;
}

void Server::setRoot(std::string &input)
{
	this->root = input;
}

bool Server::setErrorPage(std::string const &sentence)
{
	std::stringstream sstream;
	std::vector<std::string> tokens;
	std::string token;
	std::vector<int> vecErrorCode;
	std::string errorPath;

	sstream << sentence;
	while (sstream >> token)
		tokens.push_back(token);

	if (tokens.size() < 2)
		return true;
	errorPath = tokens.back();
	tokens.pop_back();

	for (size_t i = 0; i < tokens.size(); i++)
		vecErrorCode.push_back(std::strtod(tokens[i].c_str(), NULL));
	this->errorPage.insert(std::pair<std::vector<int>, std::string>(vecErrorCode, errorPath));
	return false;
}

bool Server::setRedirection(std::string const &sentence)
{
	std::stringstream sstream;
	std::vector<std::string> tokens;
	std::string token;
	std::string redirectionPath;
	int redirectionCode;

	sstream << sentence;
	while (sstream >> token)
		tokens.push_back(token);

	if (tokens.size() != 2)
		return true;
	redirectionCode = std::strtod(tokens[0].c_str(), NULL);
	redirectionPath = tokens[1];
	this->redirection[redirectionPath] = redirectionCode;
	return false;
}

void Server::setAutoIndex(std::string &input)
{
	if (input == "on")
		this->autoIndex = true;
	else
		this->autoIndex = false;
}

void Server::setClientBodySize(std::string &input)
{
	this->clientBodySize = std::strtod(input.c_str(), NULL);
}

void Location::setRoot(std::string &input)
{
	this->root = input;
}

bool Location::setErrorPage(std::string const &sentence)
{
	std::stringstream sstream;
	std::vector<std::string> tokens;
	std::string token;
	std::vector<int> vecErrorCode;
	std::string errorPath;

	sstream << sentence;
	while (sstream >> token)
		tokens.push_back(token);

	if (tokens.size() < 2)
		return true;
	errorPath = tokens.back();
	tokens.pop_back();

	for (size_t i = 0; i < tokens.size(); i++)
		vecErrorCode.push_back(std::strtod(tokens[i].c_str(), NULL));
	this->errorPage.insert(std::pair<std::vector<int>, std::string>(vecErrorCode, errorPath));
	return false;
}

bool Location::setRedirection(std::string const &sentence)
{
	std::stringstream sstream;
	std::vector<std::string> tokens;
	std::string token;
	std::string redirectionPath;
	int redirectionCode;

	sstream << sentence;
	while (sstream >> token)
		tokens.push_back(token);

	if (tokens.size() != 2)
		return true;
	redirectionCode = std::strtod(tokens[0].c_str(), NULL);
	redirectionPath = tokens[1];
	this->redirection[redirectionPath] = redirectionCode;
	return false;
}

void Location::setAutoIndex(std::string &input)
{
	if (input == "on")
		this->autoIndex = true;
	else
		this->autoIndex = false;
}

void Location::setClientBodySize(std::string &input)
{
	this->clientBodySize = std::strtod(input.c_str(), NULL);
}

bool Location::setAllowMethod(std::string &sentence)
{
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream sstream;

	sstream << sentence;
	while (sstream >> token)
		tokens.push_back(token);
	for (int i = 1; i < tokens.size(); i++)
	{
		if (tokens[i] == "GET")
			allowMethod |= GET;
		else if (tokens[i] == "POST")
			allowMethod |= POST;
		else if (tokens[i] == "PUT")
			allowMethod |= PUT;
		else if (tokens[i] == "DELETE")
			allowMethod |= DELETE;
		else
			return true;
	}
	return false;
}

void Location::setIndex(std::string &sentence)
{
	std::stringstream sstream;
	std::string token;

	sstream << sentence;
	while (sstream >> token)
		this->index.push_back(token);
}