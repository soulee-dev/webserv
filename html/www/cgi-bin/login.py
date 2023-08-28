#!/usr/bin/env python3
import cgi
import cgitb
import os
from http import cookies

cgitb.enable()

# Define hardcoded username and password for simplicity
PASSWORD = "admin"

form = cgi.FieldStorage()

# Fetch the session cookie
cookie_string = os.environ.get('HTTP_COOKIE')
session_cookie = cookies.SimpleCookie(cookie_string)

# Check if user is already logged in
if 'loggedin' in session_cookie and 'user' in session_cookie:
    print("Connection: close\r\nContent-type: text/html\r\n\r\n", end="")
    print(f"<h1>Welcome back {session_cookie['user'].value}!</h1><a href='logout.py'>Logout</a>")
    exit()

# Check login credentials
if 'username' in form and 'password' in form:
    if form['password'].value == PASSWORD:
        # Set session cookie for logged in user
        new_cookie = cookies.SimpleCookie()
        new_cookie['loggedin'] = 'yes'
        new_cookie['loggedin']['path'] = '/'
        new_cookie['user'] = form['username'].value
        new_cookie['user']['path'] = '/'
        print(new_cookie)
        print("Connection: close\r\nContent-type: text/html\r\n\r\n", end="")
        print("<h1>Login successful!</h1><a href='logout.py'>Logout</a>")
        exit()
    else:
        print("Connection: close\r\nContent-type: text/html\r\n\r\n", end="")
        print("<h1>Invalid credentials.</h1>")
        exit()
else:
    print("Connection: close\r\nContent-type: text/html\r\n\r\n", end="")
    print("""
    <form method="post" action="login.py">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username">
            <br><br>
            <label for="password">Password:</label>
            <input type="password" id="password" name="password">
            <br><br>
            <input type="submit" value="Login">
        </form>
    """)
    exit()
