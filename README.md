# Notification_Service 

Notification_Service is RESTful service, part of [Flight ticket tracking application](https://github.com/MikhailCherepanovD/notification_service).

## API Specification

### Functionality
This server provides:
* CRUD operations for users and the routes they track in the PostgreSQL database.  
* Sending HTTP requests to the Aviasales API to get up-to-date ticket information.  

### Endpoints

**Users:**  

* POST /register – register a new user.  
* POST /login – log in to the website and get user_id and user info.  
* GET /users/{user_id} – get user info.  
* PUT /users/{user_id} – update user info.  
* DELETE /users/{user_id} – delete user.  

**Routes:**  

* POST /users/{user_id}/routes – create a new route and return route_id.  
* GET /users/{user_id}/routes – get info about all the user's routes.  
* PUT /users/{user_id}/routes/{route_id} – update route info.  
* GET /users/{user_id}/routes/{route_id} – get route info.  
* DELETE /users/{user_id}/routes/{route_id} – delete route.  


**Route data:**

* GET /users/{user_id}/routes/{route_id}/current -  get  the most cheapest ticket by route now;
* GET /users/{user_id}/routes/{route_id}/cheapest - get the cheapest ticket on the route for the entire tracking time;
* GET /users/{user_id}/routes/{route_id}/statistic - get statistics on ticket for the entire tracking time;

Documentation with usage examples and server response examples can be obtained in the form of Postman tests in JSON format here: <https://github.com/MikhailCherepanovD/notification_service_tests_postman>

The Dragon framework allows you to run a server in multithreaded mode to process multiple requests simultaneously. To do this, specify the number of threads in the json.config file. File is hidden because it contains confidential information.