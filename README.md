# GPS LoRa Tracker

## Tracker Hardware Setup
1. Purchase components:

| Quantity | Item |
| ------------- | ------------- |
| 1  | Adafruit Feather M0 with LoRa radio - 900 MHz https://www.adafruit.com/product/3178  |
| 1  | Adafruit Flora Wearable Ultimate GPS module https://www.adafruit.com/product/1059  |
| 1  | Lithium Ion Polymer Battery 3.7v 500mAh https://www.adafruit.com/product/1578  |	

2. Download the .PcbDoc and .Sch files from Github
3. Export and download gerber files for printing or ordering of PCB
4. Solder components to board, for 900 MHz usage, cut 3 inches of wire and solder to ANT pad on the side of the Adafruit Feather M0. This is the antenna used for LoRa communication. There is also a pad if a uFL antenna is desired.
________________


PCB connections

| Adafruit Feather M0 | GPS Module |
| ------------- | ------------- |
| TX  | RX  |
| RX  | TX  |
| 3.3V  | 3.3V  |
| GND  | GND  |

   5. Download .STEP files from GitHub
   6. 3D print the main casing as well as the cover, use M3 bolt and nut to attach components


## Software Setup
### Tracker Side Software Setup
   1. Download the Arduino IDE to your computer, and open it after it is installed: https://www.arduino.cc/en/software
   2. Open the file “LoraTrackerNode_PowerSaving.ino” through the Arduino IDE
   3. At the top left of the IDE, click on Tools→Board→Boards Manager. In the popup window that opens, search “Adafruit SAMD Boards” and click install (the correct package should have “Adafruit Feather M0” listed as a supported board). 
   4. Then, select Tools→Board→Adafruit SAMD Boards→Adafruit Feather M0. This lets the IDE know that the code will be flashed to the Feather M0, which is the breakout board on the tracker. 
   5. Plug in the GPS tracker to your computer, and select the correct COM port through Tools→Port.
   6. Then, go to the top of the code and change the time zone difference, ping interval, and tracker ID as needed. NOTE: The tracker ID must be 0, 1, or 2. If you are setting up multiple trackers, the tracker IDs must be set to different values before flashing the code onto the tracker.
   7. Click the upload button to flash the code to the tracker.




### Gateway Side Software Setup
   1. Download the Arduino IDE to your computer, and open it after it is installed: https://www.arduino.cc/en/software
   2. Open the file “LoraGateway_PowerSaving.ino” through the Arduino IDE
   3. This next step is connecting the gateway to MongoDB, the cloud database we are using (if you have not set up MongoDB yet, go to Web Application/User Interface Software Setup for instructions)
   1. Open MongoDB in your browser and login with your credentials.
   2. At the top bar, navigate to “Realm” and then click the green button that says “Create a New App”. You can name this application anything, and then click “Create Realm Application” at the bottom
   3. In the new application you have just created, click on “3rd party services” on the left-hand side scroll panel and click “Add a Service”. Select the  “HTTP” service and name the service “gatewayAPI”. 
   4. After creating the new gatewayAPI service, click into it and click “+ Add Incoming Webhook”. We will create a webhook for POSTING information from the gateway to MongoDB.
   1. Under the settings tab, name the new webhook “POSTwebhook”. 
   2. Then, scroll down to “Webhook URL”, and copy the URL. Go back to the file “LoraGateway_PowerSaving.ino” in the Arduino IDE and paste this URL into the variable “char POSTserverName[]”. 
   3. Then, go to the function editor tab back in the POSTwebhook page, and insert the code from the file “POSTwebhook.txt” located on the github page. Click “save draft” and then deploy the webhook.
   5. Again in gatewayAPI, click “+ Add Incoming Webhook”. We will create another webook for GETTING information from MongoDB to the gateway.
   1. Under the settings tab, name the new webhook “GETwebhook”. 
   2. Then, scroll down to “Webhook URL”, and copy the URL. Go back to the file “LoraGateway_PowerSaving.ino” in the Arduino IDE and paste this URL into the variable “char GETserverName[]”. 
   3. Then, go to the function editor tab back in the GETwebhook page, and insert the code from the file “GETwebhook.txt” located on the github page. Click “save draft” and then deploy the webhook.
   4. Back in the Arduino IDE, At the top left click on Tools→Board→Boards Manager. In the popup window that opens, search “ESP32” and install the package.
   5. Then, select Tools→Board→ESP32 Arduino→T-Beam (you may have to scroll down a bit to find T-Beam). This lets the IDE know that the gateway code will be flashed to the T-Beam, which is our gateway hardware component.
   6. Plug in the T-Beam gateway to your computer, and select the correct COM port through Tools→Port.
   7. Then, enter your WIFI ssid and password into the variables at the top of the code.
   8. Click the upload button to flash the code to the gateway. After the code is flashed, you can open the serial monitor to see the printout statements made by the gateway.


### Increasing the number of supported trackers
The gateway is currently set up to handle 3 trackers. However you can copy and paste a few lines of code to support more trackers if desired. For example if you wanted to add a fourth tracker with trackerID = 3, this is how to do so:
   1. Open the file “LoraGateway_PowerSaving.ino” through the Arduino IDE
   2. In the variables at the top, create a new variable to support the ping interval for your new tracker:
String newPingInterval3 = "";
   3. In the function void loop() {} under the comment “//check if there is a request to change the tracker's ping interval”, copy and paste one of the else if {} statements: 
else if (receivedTrackerID.equals("3") && !newPingInterval3.equals("")) {
      Serial.println("sending lora signal to tracker " + receivedTrackerID + " to change the ping interval to " + newPingInterval3);
      LoRa_sendMessage(receivedTrackerID + "," + newPingInterval3);
      newPingInterval3 = "";
    }
   4. Right before the end of the void loop() {} function after the comment “//update the correct interval based on trackerID”, copy and paste one of the else if {} statements:
else if(trackerID.equals("3")) {
        newPingInterval3 = newInterval;
        Serial.print("new interval for tracker 3 ready to be set: "); Serial.println(newPingInterval3);
      }
   5. Save the file, and re-flash the code onto the T-Beam gateway (see Gateway Side Software Setup for instructions).
   6. Now, you can set up a new tracker with trackerID = 3 that will be supported by the gateway (see Tracker Side Software Setup for instructions).


## Web Application/User Interface Software Setup
1. If not downloaded already, get the most recent version of NodeJs (LTS version) for your operating system from the following website -> https://nodejs.org/en/download/
2. Next, download or clone the code from the Github repo under tracker-view
3. Before running the project, a free MongoDB cluster and Google Maps API key needs to be initialized
   1. MongoDB Cluster
      1. Create an account and sign in
      2. Create a project and initialize a cluster
      3. For the cluster, follow the default configurations for now, choosing AWS hosting and all the other default settings
      4. Once the cluster is created, click on the connection tab. For the IP address, add your device’s current one (only do this if the device is stationary or the IP will always be the same) or 0.0.0.3000 (everyone access, means anyone can access the cluster if they know the connection string)
      5. After adding the IP address, create a user and password
      6. Finally, click connect Node application and a connection string will be created. Copy that.
      7. In the codebase, go to the file named “env.local” and set the MONGODB_URI environment variable equal to the connection string. Replace the fields <user> and <password> in the connection string with the username and password create earlier
   2. Google Maps API Key
      1. Visit the following website https://developers.google.com/maps/documentation/javascript/get-api-key
      2. Scroll down a little, click on the credentials page button, and follow the instructions to create a Maps Javascript API Key
      3. Once created, copy the key string
      4. Go to the files Map.js (dir -> /components/Map.js) and analytics.js (dir -> pages/analytics.js) and fill in the Google Maps API key with your generated one
      5. For future development, create an environment variable and instead set the Maps API Key there (similar to mongodb connection string) so it is not exposed in the front end code
4. Open the command prompt to the project directory and run $npm run dev to automatically download the necessary node modules and pull in needed packages for the project
5. View the project in the development server at http://localhost:3000
6. Happy developing!!


### Web Application Deployment On Vercel
The framework used to create the web application is Next.js, and the creators of this framework have created a simple and fast deployment strategy. Before deploying, you will need to set up a repository of your choice (Github is easiest), push the source code to it, and complete step 3a and 3b from the Web Application/User Interface Software Setup section. This is because every user must maintain a unique database cluster and API key. After the setup is completed, follow these steps to deploy the Tracker View UI on Vercel
1. Visit this website https://vercel.com/new
2. Connect your repository account with Vercel and find the repo containing the code for the Tracker View UI
3. Once the codebase is selected, click import
4. On the next page, name the project what you want the website to be named (result will be https://<your project name>.vercel-app)
5. Leave the basic configurations the same
6. Under environment variables, add the MONGODB_URI under env. variable name and put the connection string for the value
7. Double check everything looks good and you’re set to deploy!
8. Vercel can update the website deployment in real time as reflected by the codebase in the selected repo. This means that pushing edits to the codebase will immediately trigger a re-deployment on vercel, so your website constantly updates alongside your codebase without any extra work on the developer’s side!
