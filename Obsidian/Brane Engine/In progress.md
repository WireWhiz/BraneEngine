Asset editing from the [[Web App]]
	Need to finish the fromJson function on assemblies ([[Assets]])
	Then change the [[Web App]] to be able to read editied values and construct a new assembly to be sent back to the server.
	Implement [[Reloading Assets]] so that the asset change shows up without the server gettting restarted.

Make it so that [[Web App]] sessions time out and cant be duplicated, possibly using a Session manager.

Networking needs to validate that connnections are actually valid, have clients try to reconnect, and probbably change back to the header based length messages.

