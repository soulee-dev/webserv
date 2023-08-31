#!/usr/bin/env python3
import os
import cgi
import cgitb
import http.cookies
import random
import hashlib
import json

cgitb.enable()  # Enables debugging for CGI

SESSION_FILE = 'sessions.json'

def set_session_id():
    """ Generate a unique session ID """
    random_data = os.urandom(16)
    session_id = hashlib.md5(random_data).hexdigest()
    return session_id

def get_session_data(session_id):
    """ Retrieve session data from the file storage """
    try:
        with open(SESSION_FILE, 'r') as f:
            data = json.load(f)
            return data.get(session_id, {})
    except (FileNotFoundError, json.JSONDecodeError):
        return {}

def save_session_data(session_id, data):
    """ Save session data to the file storage """
    try:
        with open(SESSION_FILE, 'r') as f:
            all_data = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        all_data = {}

    all_data[session_id] = data

    with open(SESSION_FILE, 'w') as f:
        json.dump(all_data, f)

def main():
    form = cgi.FieldStorage()

    # Check for the session cookie
    if 'HTTP_COOKIE' in os.environ:
        cookie_string = os.environ.get('HTTP_COOKIE')
        cookies = http.cookies.SimpleCookie()
        cookies.load(cookie_string)
    else:
        cookies = http.cookies.SimpleCookie()

    if 'session' not in cookies:
        session_id = set_session_id()
        cookies['session'] = session_id
        cookies['session']['path'] = '/'
        session_data = {}
    else:
        session_id = cookies['session'].value
        session_data = get_session_data(session_id)

    # Example of storing a new key-value in the session
    key = form.getvalue("key", "")
    value = form.getvalue("value", "")
    if key and value:
        session_data[key] = value
        save_session_data(session_id, session_data)

    print(cookies)
    print("Content-type: text/html\r\n\r\n", end="")
    print("<html><body><h1>Welcome</h1>")
    print(f"<p>Your session ID is: {session_id}</p>", end="")
    for k, v in session_data.items():
        print(f"<p>{k}: {v}</p>", end="")
    print("</body></html>\r\n\r\n", end="")

if __name__ == "__main__":
    main()
