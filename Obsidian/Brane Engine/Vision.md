# Name
Brane engine is a reference to the string theory concept of a membrane. TLDR Brane Engine = Universe Engine with lots of strings and connections
# Purpose
Provide a framework for XR creators to be able to create and share high performance and fully fleshed out experiences without being beholden to any platform. To accomplish this goal I'll eventually have to make it open source, probbably once I release my first demo, so I feel more comfortable about the quality of my code.

# Description 
The engine will work like a web browser, you will connect to an asset server through a domain, which will promt your [[Client]] to use one of it's [[Runtime Sever]]s. Than your client will download any [[Assets]] it needs from [[Asset Server]]s as you play. There will be support for assets such as meshes and textures to load in asynchonusly, so you can see the parts of the mesh appear as they load in. Because of the very modular approch that the engine is going to take, things such as full open worlds should be possible in the long term.

The reason that I'm not just using WebXR, even though it its very similar is because I feel like it better fulfills the need for the engine to not be reliant upon other technologies, such as a web browser, that could limit it's potential. Unfortunatly I still need to use JavaScript for the [[Web App]]

The engine is going to use an [[ECS]] based system to store data and run systems, as I feel that it goes well with the modular nature of what I'm trying to create. It also simplifies the creation of user created data types and scripts. (Long term, took forever to get the basic ECS working) 

[[Assets]] will be able to be stored on multiple [[Asset Server]]s, and even used accross different worlds. It's just a matter of the devs making sure that the scripts and objects work together nicely. I feel like this functionality will be moslty utilized in social settings where game ballance is not important and people just want to have fun with cool custom avatars.