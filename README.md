# Route Manager

## Description

Route Manager is a C++ application designed to manage bus routes and stops. It allows users to:
* Record information about bus routes, stops, and their connections.
* Calculate the optimal route between any two stops based on time.
* Retrieve information about specific buses or stops.
* Render visual maps of the bus routes in SVG format.

The application takes input in JSON format to define settings, base data (stops and buses), and query requests. It outputs the results of the queries, also in JSON format.

## Features

* **Bus and Stop Management**: Add buses with their routes (circular or linear) and stops with geographic coordinates and road distances.
* **Route Calculation**: Finds the minimum time route between two specified stops, considering bus wait times and travel velocity.
* **Information Retrieval**: Provides details about specific bus routes (length, stop count, unique stops, curvature) and stops (buses serving the stop).
* **Map Rendering**: Generates SVG maps visualizing the bus routes, stops, and calculated paths. Map elements like lines, labels, and points are configurable.
* **Coordinate Handling**: Uses spherical coordinates (latitude, longitude) for stops and calculates distances accordingly. Includes sophisticated coordinate compression for map rendering.

## Dependencies

The project relies on the following external libraries:

* **googletest**: For unit testing. Fetched via CMake's `FetchContent` from [https://github.com/google/googletest.git](https://github.com/google/googletest.git).
* **svg**: A library for generating SVG graphics. Fetched via CMake's `FetchContent` from [https://github.com/siarheishut/svg.git](https://github.com/siarheishut/svg.git).
* **json**: A JSON library (included as `lib/json`).
* **graph**: A graph library (included as `lib/graph`).

## Building

The project uses CMake for building. Ensure you have CMake (version 3.24 or higher) and a C++17 compatible compiler installed.

1.  **Clone the repository (if applicable)**
2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```
3.  **Configure with CMake:**
    ```bash
    cmake ..
    ```
4.  **Build the project:**
    ```bash
    cmake --build .
    ```
    This will create the `root_manager` executable.

## Input Format

Input is provided as a single JSON map via standard input. The map contains the following top-level fields:

| Field            | Type  | Optional | Description                           |
| :--------------- | :---- | :------- | :------------------------------------ |
| routing_settings | map   | No       | Configuration for route calculation.  |
| render_settings  | map   | No       | Configuration for map rendering.      |
| base_requests    | array | No       | Requests to initialize the database.  |
| stat_requests    | array | No       | Queries to retrieve information.      |

---

### `routing_settings`

| Field         | Type  | Optional | Description                                            |
| :------------ | :---- | :------- | :----------------------------------------------------- |
| bus_wait_time | int   | No       | Waiting time for a bus at any stop (in minutes).       |
| bus_velocity  | float | No       | Assumed constant bus speed (in kilometers per hour). |

> **Assumptions:**
> * Passengers wait exactly `bus_wait_time` minutes upon arrival at any stop for any bus.
> * All buses travel at a constant speed equal to `bus_velocity`.

---

### `render_settings`

| Field                | Type       | Optional | Description                                                                                                                                                                       |
| :------------------- | :--------- | :------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| width                | float      | No       | Width of the output SVG map.                                                                                                                                                      |
| height               | float      | No       | Height of the output SVG map.                                                                                                                                                     |
| padding              | float      | No       | Padding space from the map edges.                                                                                                                                                 |
| stop_radius          | float      | No       | Radius of the circle representing a stop on the map.                                                                                                                              |
| line_width           | float      | No       | Width of the lines representing bus routes.                                                                                                                                       |
| stop_label_font_size | int        | No       | Font size for stop labels.                                                                                                                                                      |
| stop_label_offset    | array      | No       | `[x, y]` offset (floats) for stop name labels relative to the stop point.                                                                                                         |
| underlayer_color     | svg::Color | No       | Color of the underlayer/outline for labels (can be string like "red" or array `[r, g, b]` or `[r, g, b, a]`).                                                                      |
| underlayer_width     | float      | No       | Width of the underlayer/outline lines for labels.                                                                                                                                 |
| color_palette        | array      | No       | Array of `svg::Color` used cyclically for different bus routes.                                                                                                                   |
| bus_label_font_size  | int        | No       | Font size for bus name labels.                                                                                                                                                  |
| bus_label_offset     | array      | No       | `[x, y]` offset (floats) for bus name labels relative to route endpoints.                                                                                                       |
| layers               | array      | No       | Order of elements to draw on the map. Valid strings: `"bus_lines"`, `"bus_labels"`, `"stop_points"`, `"stop_labels"`.                                                              |
| outer_margin         | float      | No       | Outer margin used for route-specific map rendering (added in `src/request_parser.cpp`, seems related to highlighting routes).                                                     |

---

### `base_requests`

An array of JSON maps, each describing either a stop or a bus to add to the database.

1.  **Stop Request (`"type": "Stop"`)**: Adds a stop definition.

    | Field          | Type   | Optional | Description                                                                                             |
    | :------------- | :----- | :------- | :------------------------------------------------------------------------------------------------------ |
    | type           | string | No       | Must be `"Stop"`.                                                                                       |
    | name           | string | No       | Unique name of the stop.                                                                                |
    | latitude       | float  | No       | Latitude coordinate of the stop (degrees).                                                              |
    | longitude      | float  | No       | Longitude coordinate of the stop (degrees).                                                             |
    | road_distances | map    | No       | Map where keys are names of other stops and values are road distances (integers, in meters) *from* this stop *to* the key stop. |

    > **Distance Notes:**
    > * If distance `A -> B` is given but `B -> A` is not, the distance `B -> A` is assumed to be equal to `A -> B`.
    > * If neither `A -> B` nor `B -> A` is provided, the distance is calculated based on geographical coordinates (Haversine formula).

2.  **Bus Request (`"type": "Bus"`)**: Adds a bus route definition.

    | Field        | Type   | Optional | Description                                                  |
    | :----------- | :----- | :------- | :----------------------------------------------------------- |
    | type         | string | No       | Must be `"Bus"`.                                             |
    | name         | string | No       | Unique name of the bus.                                      |
    | stops        | array  | No       | Array of stop names (strings) defining the sequence of stops. |
    | is_roundtrip | bool   | No       | `true` for circular routes, `false` for linear routes.       |

    > **Route Types:**
    > * If `is_roundtrip` is `true`, the route `["A", "B", "C", "A"]` goes `A -> B -> C -> A` and repeats. The first and last stops in the `stops` array must be the same.
    > * If `is_roundtrip` is `false`, the route `["A", "B", "C"]` goes `A -> B -> C -> B -> A` and repeats.

---

### `stat_requests`

An array of JSON maps, each representing a query to the database.

1.  **Bus Info Request (`"type": "Bus"`)**: Get details about a bus route.

    | Field | Type   | Optional | Description                 |
    | :---- | :----- | :------- | :-------------------------- |
    | id    | int    | No       | Unique request identifier.  |
    | type  | string | No       | Must be `"Bus"`.            |
    | name  | string | No       | Name of the bus to query. |

2.  **Stop Info Request (`"type": "Stop"`)**: Get details about a stop.

    | Field | Type   | Optional | Description                 |
    | :---- | :----- | :------- | :-------------------------- |
    | id    | int    | No       | Unique request identifier.  |
    | type  | string | No       | Must be `"Stop"`.           |
    | name  | string | No       | Name of the stop to query. |

3.  **Route Request (`"type": "Route"`)**: Calculate the best route between two stops.

    | Field | Type   | Optional | Description                 |
    | :---- | :----- | :------- | :-------------------------- |
    | id    | int    | No       | Unique request identifier.  |
    | type  | string | No       | Must be `"Route"`.          |
    | from  | string | No       | Name of the starting stop.  |
    | to    | string | No       | Name of the destination stop. |

4.  **Map Request (`"type": "Map"`)**: Request the full map rendering.

    | Field | Type   | Optional | Description                 |
    | :---- | :----- | :------- | :-------------------------- |
    | id    | int    | No       | Unique request identifier.  |
    | type  | string | No       | Must be `"Map"`.            |

## Output Format

Output is a JSON array containing responses corresponding to each request in `stat_requests`, maintaining the order. Each response is a JSON map.

---

1.  **Bus Info Response**: Response to a `"Bus"` stat request.

    * *If found:*
        | Field             | Type  | Description                                                                                          |
        | :---------------- | :---- | :--------------------------------------------------------------------------------------------------- |
        | request_id        | int   | ID matching the request.                                                                             |
        | route_length      | float | Total road distance of the route in meters.                                                        |
        | stop_count        | int   | Total number of stops visited in one full cycle (including returns for linear routes).             |
        | unique_stop_count | int   | Number of unique stops on the route.                                                                 |
        | curvature         | float | Ratio of `route_length` to the geographical distance calculated directly between consecutive stops. |
    * *If not found:*
        | Field         | Type   | Description                              |
        | :------------ | :----- | :--------------------------------------- |
        | request_id    | int    | ID matching the request.                 |
        | error_message | string | `"not found"`                            |

---

2.  **Stop Info Response**: Response to a `"Stop"` stat request.

    * *If found:*
        | Field      | Type  | Description                                                               |
        | :--------- | :---- | :------------------------------------------------------------------------ |
        | request_id | int   | ID matching the request.                                                  |
        | buses      | array | Array of bus names (strings) that pass through this stop, sorted alphabetically. |
    * *If not found:*
        | Field         | Type   | Description                              |
        | :------------ | :----- | :--------------------------------------- |
        | request_id    | int    | ID matching the request.                 |
        | error_message | string | `"not found"`                            |

---

3.  **Route Response**: Response to a `"Route"` stat request.

    * *If route exists:*
        | Field      | Type   | Description                                                                     |
        | :--------- | :----- | :------------------------------------------------------------------------------ |
        | request_id | int    | ID matching the request.                                                        |
        | total_time | float  | Minimum time (in minutes) required to travel from `from` stop to `to` stop.   |
        | items      | array  | Array detailing the steps of the journey (see below).                         |
        | map        | string | SVG representation of the route overlaid on the map.                            |
    * *If no route exists:*
        | Field         | Type   | Description                              |
        | :------------ | :----- | :--------------------------------------- |
        | request_id    | int    | ID matching the request.                 |
        | error_message | string | `"not found"`                            |

    **Route `items`**: Each item describes a segment of the journey.
    * **Wait Item (`"type": "Wait"`)**: Waiting at a stop.
        | Field     | Type   | Description                        |
        | :-------- | :----- | :--------------------------------- |
        | type      | string | `"Wait"`                           |
        | stop_name | string | Name of the stop to wait at.     |
        | time      | int    | Waiting time (equals `bus_wait_time`). |
    * **Bus Item (`"type": "Bus"`)**: Traveling on a bus.
        | Field      | Type   | Description                        |
        | :--------- | :----- | :--------------------------------- |
        | type       | string | `"Bus"`                            |
        | bus        | string | Name of the bus taken.             |
        | span_count | int    | Number of stops traveled on this bus segment. |
        | time       | double | Travel time (in minutes) for this segment. |

---

4.  **Map Response**: Response to a `"Map"` stat request.
    | Field      | Type   | Description                 |
    | :--------- | :----- | :-------------------------- |
    | request_id | int    | ID matching the request.    |
    | map        | string | SVG representation of the full bus network map. |

## Testing

The project includes unit tests using the Google Test framework. Tests cover various components including:
* Request Parsing (`request_parser_test.cpp`)
* Settings Parsing (`settings_parser_test.cpp`)
* Bus Management (`bus_manager_test.cpp`)
* Request Processing (`request_processor_test.cpp`)
* Color Parsing (`color_parser_test.cpp`)
* Coordinate Conversion (`coords_converter_test.cpp`)
* Map Rendering (`map_renderer_test.cpp`)

You can run the tests using the `route_manager_tests` executable generated during the build.
