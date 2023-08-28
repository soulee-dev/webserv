#!/usr/bin/env python3

import cgi
import cgitb
import sys
import os

cgitb.enable()  # Enable debugging

form = cgi.FieldStorage()

# Get filename
fileitem = form['filename']

# Test if the file was uploaded
if fileitem.filename:
    # strip leading path from file name to avoid directory traversal attacks
    filename = os.path.basename(fileitem.filename)
    with open('./html/www/uploaded_files/' + filename, 'wb') as file:
        file.write(fileitem.file.read())

    message = 'The file "' + filename + '" was uploaded successfully'
else:
    message = 'No file was uploaded'

print("""Connection: close\r\nContent-Type: text/html\r\n\r\n""", end="")

print("""\
<html>
<head>
    <title>File Upload Result</title>
</head>
<body>
    <h1>{}</h1>
</body>
</html>""".format(message))
