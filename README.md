# FTP-Client
A Simple FTP Client that supports a subset of commands defined by [RFC959](https://www.ietf.org/rfc/rfc959.txt) and one from [RFC2389](https://tools.ietf.org/html/rfc2389#section-3.1)

## Commands Supported

| Command       | Description    
| ------------- |:-------------:|
| USER `<user>`   |  Set or change user|
| PASS `<pass>`    | Send the password|
| GET `<file>` | Retrieves the `<file>` from the ftp server    |
| DELETE `<file>` | Delete the file `<file>` from the ftp server     |
| CD `<path>`| Change directory to `<path> `    |
| NLIST `<path>?` | Lists files in `<path>` or current directory if no argument is provided      | 
| PWD | Print current directory      |
| FEATURES | List ftp server features      |
| QUIT | Ends session and closes the client      |
