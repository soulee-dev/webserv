#!/usr/bin/env python3

import cgi
import cgitb
from http import cookies

cgitb.enable()

# Delete the session cookie
cookie = cookies.SimpleCookie()
cookie['loggedin'] = ''
cookie['loggedin']['expires'] = 'Thu, 01 Jan 1970 00:00:00 GMT'
cookie['loggedin']['path'] = '/'
cookie['user'] = ''
cookie['user']['expires'] = 'Thu, 01 Jan 1970 00:00:00 GMT'
cookie['user']['path'] = '/'
print(cookie)
print("Content-type: text/html\r\n\r\n")
print("""
    <h1>Logged out!</h1>
    <a href="login.py">Go back to login</a>
""")
exit()