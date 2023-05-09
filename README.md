## Route manager

Bus Route Manager allows you to record information about various buses with their routes and stops, and later receive
information about it.

### Input

Input is represented as a JSON map with the following fields:

| Field         | Type  | Optional | Description                           |  
|---------------|-------|----------|---------------------------------------|  
| base_requests | array | No       | Requests for initializing a database. |   
| stat_requests | array | No       | Requests to the database.             |           

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