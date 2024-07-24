# _database

A super simple document database of JSON documents

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

You create a database by defining a schema.

request body:

``` json
{
    "inventor": "",
    "colors": [""],
    "flavors": [""],
    "size": {
        "width": 0,
        "height": 0
    },
    "rating": 0,
}
```

### POST /upsert?db=desserts&id=cake

You upsert some stuff into that database, the whole document at a time.

request body:

``` json
{
    "inventor": "some ancient Egyptian dude",
    "colors": ["red", "white", "blue"],
    "flavors": ["chocolate", "vanilla", "strawberry"],
    "size": {
        "width": 100,
        "height": 200
    },
    "rating": 10,
}
```

### POST /upsert?db=desserts&id=cake.colors

Or upserting an update via JSON data's dot notation.

request body:

``` json
["red", "white", "blue"]
```

### POST /upsert?db=desserts&id=cake.size.width

Dot notation works as expected ad infinitum.

request body:

``` json
100
```

### GET /doc?db=desserts&id=cake

You can get that whole doc directly by it's id.

response body:

``` json
{
    "colors": ["red", "white", "blue"],
    "flavors": ["chocolate", "vanilla", "strawberry"],
    "size": {
        "width": 100,
        "height": 200
    },
    "rating": 10,
}
```

### GET /doc?db=desserts&id=cake.colors

Or by dot notation.

response body:

``` json
["red", "white", "blue"]
```

### GET /doc?db=desserts&id=cake.size.width

Dot notation ad infinitum.

response body:

``` json
100
```

### DELETE /delete?db=dessert&id=cake

You can also remove documents. (The flagship feature!)

### POST /index/upsert?db=desserts

You can get a little more fancy with creating indexes on document properties.

request body:

``` json
["inventor", "rating"]
```

### GET /find?db=desserts&key=rating&value=10

Which you can then find directly by property and value.

request body:

``` json
[
  {
    "cake": {
      "colors": ["red", "white", "blue"],
      "flavors": ["chocolate", "vanilla", "strawberry"],
      "size": {
          "width": 100,
          "height": 200
      },
      "rating": 10,
    }
  }
]
```
