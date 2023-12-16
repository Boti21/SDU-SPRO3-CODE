// Convert html to C string: https://tomeko.net/online_tools/cpp_text_escape.php?lang=en  
#define FORKCONNECThtml  "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n        <meta charset=\"UTF-8\">\n        <meta http-equiv=\"refresh\" content=\"1\">\n        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n        <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n        <title>Forklift connect</title>\n        <style>\n            \n             h1 {\n                color:black;\n                font-style: italic;\n                font-size: 30px;\n                text-decoration: underline;\n            \n             }\n\n            * {\n                box-sizing: border-box;\n                margin:0;\n                padding:0;\n                font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif;\n            }\n\n            nav{\n                 height:80px;\n                 background-color: coral;\n                 display: flex;\n                 justify-content: space-between;\n                 align-items: center;\n                    padding: 0rem calc((100vw - 1300px) / 2);\n            }\n\n            .logo{\n                color: white;\n                font-size: rem;\n                font-weight: bold;\n                font-style: italic;\n                padding: 0 2 rem;\n            }\n\n            nav a {\n                text-decoration: wavy;\n                color:white;\n                padding: 0 1.5rem;\n                font-weight: bold;\n                text-transform: uppercase;\n            }\n\n            .hero{\n                background-color:rgba(166, 216, 216, 0.927);\n            }\n\n            .hero-container{\n                display: grid;\n                grid-template-columns: 1fr 1fr;\n                height: 95vh;\n                padding: 3rem calc((100vw - 1300px) / 2);\n\n            }\n\n            .column-left{\n                display: flex;\n                flex-direction:column;\n                justify-content: center;\n                align-items: flex-start;\n                color:black;\n                padding: 0rem 2rem;\n            }\n\n            .column-left h1{\n                margin-bottom: 1rem;\n                font-size: 3rem;   \n            }\n\n            .column-left p {\n                margin-bottom: 2rem;\n                font-size: 1.5rem;\n                        line-height: 1.5;   \n                    }\n\n            .button1 {\n                padding: 1rem 3rem;\n                font-size: 1rem;\n                text-align: center;\n                font-weight: bold;\n                border:none;\n                color:azure;\n                background-color: red;\n                cursor:pointer;\n                border-radius: 50px;\n                position:relative;\n\n            }\n\n            .button2 {\n                padding: 1rem 3rem;\n                font-size: 1rem;\n                text-align: center;\n                font-weight: bold;\n                border:none;\n                color:azure;\n                background-color: green;\n                cursor:pointer;\n                border-radius: 50px;\n                position:relative;\n            }\n\n            button:hover {\n                background-color: white;\n                color: black;\n\n            }\n\n            .column-right{\n            display:flex;\n            justify-content: center;\n            align-items: center;\n            padding: 0rem 2rem;\n            }\n\n            label {\n            font-size: 25px; /* Adjust the size as needed */\n            font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif; /* Use Arial or a similar font */\n            }\n            \n\n            </style>\n            \n            \n</head>\n<body>\n\n    <nav> \n        <div class=\"logo\"> FORK CONNECT </div> \n        <div class=\"nav-items\">\n            <a>Home</a>\n            <a>About</a>\n            <a>Contact</a>\n        </div>\n    </nav>\n\n    <section class=\"hero\">\n        <div class=\"hero-container\">\n            <div class=\"column-left\">\n                <h1>Connect to the forklift!</h1>\n                \n                <p> You can start or stop the forklift pressing the buttons:</p>\n                <button class=\"button button1\" onclick=\"window.location.href='/start'\"> START </button>\n                <button class=\"button button2\" onclick=\"window.location.href='/stop'\"> STOP </button>\n                \n\n            <p> You can choose a pallet and place it one of our fields:</p>\n            \n            <form action=\"/drop\" method=\"post\">\n\n                <label for=\"dropdown1\"> Choose pallet:</label>\n                <select id=\"dropdown1\" name=\"dropdown1\">\n                    <option value=\"option1\"> pallet 1</option>\n                    <option value=\"option2\"> pallet 2</option>\n                </select>\n\n                <label for=\"dropdown2\"> Choose field:</label>\n                <select id=\"dropdown2\" name=\"dropdown2\">\n                    <option value=\"choiceA\"> Field A</option>\n                    <option value=\"ChoiceB\"> Field B</option>\n                </select>\n\n                <input type=\"submit\" value=\"Submit\">\n                \n            </form>\n            <p> Updates of the forklift:</p>\n            <p> %s </p>\n            </div>\n              \n    </section> \n    \n</body>\n</html>\n\n\n\n\n\n\n\n\n\n"