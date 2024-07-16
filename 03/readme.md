# _database

A stupid simple document database of JSON documents (with comments too!)

## Settings

settings.json

-port: The port the database should listen at.
-auth: An authorization key to help prevent bad guys doing bad things.
    *must be at least 32 characters long, suggested to be at least 64*
-ip: The single IP that is allowed to connect to this server.
-path: The root path to store the data at.

``` json
{
  "port": 7777,
  "auth": "CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS",
  "ip": "127.0.0.1",
  "path": "./"
}
```
