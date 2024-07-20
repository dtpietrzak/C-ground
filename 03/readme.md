# _database

A stupid simple document database of JSON documents

## Settings

settings.json

- port: The port the database should listen at.
- auth: An authorization key to help prevent bad guys doing bad things.
  - *must be at least 32 characters long, suggested to be at least 64*
- ip: The single IP that is allowed to connect to this server.
- path: The root path to store the data at.

``` json
{
  "port": 7777,
  "auth": "CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS",
  "ip": "127.0.0.1",
  "path": "./"
}
```

## Generic Response

```js
{
  "status": 200,
  "body": ...response_body
}
```

## Routes

### POST /schema?db=desserts

request body:

``` json
{
    "colors": [""],
    "flavors": [""],
    "size": {
        "width": 0,
        "height": 0
    }
}
```

### POST /upsert?db=desserts&id=cake

request body:

``` json
{
    "colors": ["red", "white", "blue"],
    "flavors": ["chocolate", "vanilla", "strawberry"],
    "size": {
        "width": 100,
        "height": 200
    }
}
```

### POST /upsert?db=desserts&id=cake.colors

request body:

``` json
["red", "white", "blue"]
```

### POST /upsert?db=desserts&id=cake.size.width

request body:

``` json
100
```

### GET /docs?db=desserts&id=cake

response body:

``` json
{
    "colors": ["red", "white", "blue"],
    "flavors": ["chocolate", "vanilla", "strawberry"],
    "size": {
        "width": 100,
        "height": 200
    }
}
```

### GET /docs?db=desserts&id=cake.colors

response body:

``` json
["red", "white", "blue"]
```

### GET /docs?db=desserts&id=cake.size.width

response body:

``` json
100
```

### DELETE /delete?db=dessert&id=cake
