## Route manager

Bus Route Manager allows you to record information about various buses with their routes and stops, and later receive
information about any stop, bus or a route between any two stops.

### Input

Input is represented as a JSON map with the following fields:

| Field            | Type  | Optional | Description                           |  
|------------------|-------|----------|---------------------------------------|  
| routing_settings | map   | No       | Route config.                         |           
| render_settings  | map   | No       | Config for map rendering.             |           
| base_requests    | array | No       | Requests for initializing a database. |   
| stat_requests    | array | No       | Requests to the database.             |           

#### routing_settings:

| Field         | Type  | Optional | Description                                            |  
|---------------|-------|----------|--------------------------------------------------------|  
| bus_wait_time | int   | No       | Waiting time for the bus at the bus stop (in minutes). |           
| bus_velocity  | float | No       | The bus speed (in kilometers per hour).                |   

> The algorithm assumes that whenever a person arrives at a stop and whatever that stop is,
> they will wait for any bus for exactly `bus_wait_time` minutes.
>
> The algorithm assumes that the speed of any bus is constant and exactly equal to `bus_velocity`.

#### render_settings:

| Field                | Type       | Optional | Description                                                                                                                                                                       |  
|----------------------|------------|----------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|  
| width                | float      | No       | Map width.                                                                                                                                                                        |           
| height               | float      | No       | Map height.                                                                                                                                                                       |           
| padding              | float      | No       | Indentation from the map edges.                                                                                                                                                   |           
| stop_radius          | float      | No       | Stop circle radius.                                                                                                                                                               |           
| line_width           | float      | No       | Route line width.                                                                                                                                                                 |           
| stop_label_font_size | int        | No       | Stop label font size.                                                                                                                                                             |           
| stop_label_offset    | array      | No       | Array of 2 floats. x offset and y offset for stop names.                                                                                                                          |           
| bus_label_font_size  | int        | No       | Bus label font size.                                                                                                                                                              |           
| bus_label_offset     | array      | No       | Array of 2 floats. x offset and y offset for bus names.                                                                                                                           |           
| underlayer_color     | svg::Color | No       | The color of the underlay under the labels.                                                                                                                                       |           
| underlayer_width     | float      | No       | The width of the underlay lines under the labels.                                                                                                                                 |           
| color_palette        | array      | No       | Route color palette.                                                                                                                                                              |           
| layers               | array      | No       | Array of strings. Layers to be printed on the map in response to Route request and Map Request. Layers can be as follows: "bus_lines", "bus_labels, "stop_points", "stop_labels". |           

#### base_requests:

Each base_request in the array is a JSON map of one of the following types:

1. Bus request.

   Adds a bus with its route to the database.

| Field        | Type   | Optional | Description                                              |  
|--------------|--------|----------|----------------------------------------------------------|  
| type         | string | No       | Must be equal to "Bus".                                  |  
| name         | string | No       | The name of the bus. Must be unique.                     |  
| stops        | array  | No       | The route of the bus represented as an array of strings. | 
| is_roundtrip | bool   | No       | Indicates a circular route.                              |  

> if `is_roundtrip` is set to `true`, than `"stops" : ["stop1", "stop2", "stop3", "stop1"]` describes the route, that
> starts at `stop1`, then goes to `stop2`, `stop3`, and then returns back directly to `stop1`. Then this route
> repeats: `stop1 -> stop2 -> stop3 -> stop1`.
>
> if `is_roundtrip` is set to `false`, then `"stops" : ["stop1", "stop2", "stop3"]` describes the route, that starts
> at `stop1`, then goes to `stop2`, `stop3`, then back to `stop2` and finally returns to `stop1`. Then this route
> repeats: `stop1 -> stop2 -> stop3 -> stop2 -> stop1`.
>
> Note: if `is_roundtrip` is set to `true`, the first stop must be the last stop.

2. Stop request.

   Adds a stop with its coordinates and road lengths to the database.

| Field          | Type   | Optional | Description                                                                                             |  
|----------------|--------|----------|---------------------------------------------------------------------------------------------------------|  
| type           | string | No       | Must be equal to "Stop".                                                                                |  
| name           | string | No       | The name of the stop. Must be unique.                                                                   |  
| latitude       | float  | No       | The latitude of the stop.                                                                               | 
| longitude      | float  | No       | The longitude of the stop.                                                                              |  
| road_distances | map    | No       | Pairs {`stopX`, `distX`}, where integer `distX` is a road length in meters from stop `name` to `stopX`. |  

> The distance from `A` to `B` is considered to be equal to the distance from `B` to `A`, if road length
> from `A` to `B` is not provided explicitly.
>
> If neither `A`->`B` nor `B`->`A` road lengths are provided, the distance between the stops is computed by coordinates.

#### stat_requests:

Each stat_request in the array is a JSON map of one of the following types:

1. Bus request.

| Field | Type   | Optional | Description                                         |  
|-------|--------|----------|-----------------------------------------------------|  
| id    | int    | No       | Request ID. Must be unique.                         |  
| type  | string | No       | Must be equal to "Bus".                             | 
| name  | string | No       | The name of the bus for which you want to get info. |

2. Stop request.

| Field | Type   | Optional | Description                                          |  
|-------|--------|----------|------------------------------------------------------|  
| id    | int    | No       | Request ID. Must be unique.                          |  
| type  | string | No       | Must be equal to "Stop".                             |  
| name  | string | No       | The name of the stop for which you want to get info. |

3. Route request.

| Field | Type   | Optional | Description                    |  
|-------|--------|----------|--------------------------------|  
| id    | int    | No       | Request ID. Must be unique.    |  
| type  | string | No       | Must be equal to "Route".      |  
| from  | string | No       | The name of the starting stop. |
| to    | string | No       | The name of the finish stop.   |

4. Map request.

| Field | Type   | Optional | Description                 |  
|-------|--------|----------|-----------------------------|  
| id    | int    | No       | Request ID. Must be unique. |  
| type  | string | No       | Must be equal to "Map".     |  

### Output:

The output is represented as a JSON array, where each element is a response to a related stat_request. Each response is
a JSON map of one of the following types:

1. Response to Bus Request.

- `If the bus was successfully found:`

| Field             | Type  | Description                                                                                                                                              |  
|-------------------|-------|----------------------------------------------------------------------------------------------------------------------------------------------------------|  
| route_length      | float | The length of the bus route in meters.                                                                                                                   |  
| request_id        | int   | An ID of the corresponding request.                                                                                                                      |  
| stop_count        | int   | A number of stops on the bus route.                                                                                                                      |  
| unique_stop_count | int   | A number of unique stops on the bus route.                                                                                                               |  
| curvature         | float | `route_length` / `geo_route_length`. `geo_route_length` is an ideal route length: distance between any stopX and stopY is computed by their coordinates. |

- `If the bus wasn't found:`

| Field         | Type   | Description                                      |  
|---------------|--------|--------------------------------------------------| 
| request_id    | int    | An ID of the corresponding request.              | 
| error_message | string | An error message, which is equal to "not found". | 

2. Response to Stop Request.

- `If the stop is present in the database:`

| Field      | Type  | Description                                                                    |  
|------------|-------|--------------------------------------------------------------------------------|  
| buses      | array | All the buses that stop at the provided stop, listed in lexicographical order. |  
| request_id | int   | An ID of the corresponding request.                                            |

- `If the stop is not in the database:`

| Field         | Type   | Description                                      |  
|---------------|--------|--------------------------------------------------|  
| request_id    | int    | An ID of the corresponding request.              |  
| error_message | string | An error message, which is equal to "not found". |  

3. Response to Route Request.

- `If there is no route between such two stops:`

| Field         | Type   | Description                                      |  
|---------------|--------|--------------------------------------------------| 
| request_id    | int    | An ID of the corresponding request.              | 
| error_message | string | An error message, which is equal to "not found". | 

- `If exists at least one route between such two stops:`

| Field      | Type   | Description                                                                               |  
|------------|--------|-------------------------------------------------------------------------------------------|  
| request_id | int    | An ID of the corresponding request.                                                       |  
| total_time | float  | The minimum time required to travel from stop from to stop to.                            | 
| items      | array  | List of route elements, each of which describes the continuous activity of the passenger. | 
| map        | string | Route on the map. Presented in SVG format. For the map coordinates compression is used.   |

There are 2 types of items:

Wait(Wait at the stop):

| Fields    | Type   | Description                                  |
|-----------|--------|----------------------------------------------|
| type      | string | Equal to "Wait".                             |
| stop_name | string | The stop to wait at.                         |
| time      | int    | The waiting time (equal to `bus_wait_time`). |

Bus(Take a bus at the stop):

| Fields     | Type   | Description                    |
|------------|--------|--------------------------------|
| type       | string | Equal to "Bus".                |
| bus        | string | The bus to get on.             |
| span_count | int    | The number of stops to travel. |
| time       | double | The travel time.               |

4. Response to Map Request.

Map with a single key "map" with a map in SVG format. For the map coordinates compression is used.
