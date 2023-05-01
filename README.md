## Route manager

### How to use

Bus Route Manager allows you to record information about various buses with their routes and stops, and later receive
information about it. Input format:

**1.**  A line with a non-negative integer number N - the number of base requests.  
**2.**  N lines with base requests. One request per line.  
**3.**  A line with a non-negative integer number K - the number of stat requests.  
**4.**  K lines with stat requests. One request per line. Each line must end on '\n'.

##

### Base requests:

1. Bus {**bus**}: {**stop1**} - {**stop2**} - ... - {**stopn**}  
   Adds a bus with a non-circular route to the database. The route will be expanded to the following form: **{stop1},
   {stop2}, ..., {stopn}, {stopn-1}, ... ,{stop1}.**

2. Bus {**bus**}: {**stop1**} > {**stop2**} > ... > {**stopn**} > {**stop1**}  
   Adds a bus with a circular route to the database. The first and the last stops on the road must be identical.

3. Stop {**stop**}: {**latitude**}, {**longitude**}, {**dist1**}m to {**stop1**}, {**dist2**}m to {**stop2**}, ...  
   Adds a stop, its coordinates and road lengths from {**stop**} to {**stopX**}.

   > Distance from {stopX} to {stop} = distance from {stop} to {stopX}, if road_length from {stopX} to {stop} wasn’t set
   explicitly.
   >
   > If neither {stopX}-{stopY} nor {stopY}-{stopX} road_lengths were provided, the geo_distance (computed by
   coordinates) will be considered the distance between the stops.

##

### Stat requests:

**1.**  Bus {**bus**}.  
**2.**  Stop {**stop**}.

Responses to stat requests:

1.
    - `If the bus was successfully found:`  
      Bus {**bus**}: {**stop_count**} stops on route, {**unique_stop_count**} unique stops, {**route_length**} route
      length, {**curvature**} curvature.

      > curvature = route_length / geo_route_length.  
      > geo_route_length is an ideal route length: distance between any stopX and stopY is computed by their
      coordinates.

    - `If the bus wasn't found:`  
      Bus {**bus**}: not found.

2.
    - `If this stop is in the database, and it’s included in at least one route:`  
      Stop {**stop**}: buses {**bus1**} {**bus2**} ... {**busn**}
      > The buses are listed in lexicographical order.

    - `If the stop is in the database, but it’s not included in any route:`  
      Stop {**stop**}: no buses

    - `If the stop is not in the database:`  
      Stop {**stop**}: not found

