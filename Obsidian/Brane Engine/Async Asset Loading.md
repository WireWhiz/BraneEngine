A core feature of the engine will be the ability to load in assets aysnchonusly, this means that you don't need to sit in a void as things load in, but can instead watch it load in around you, all the way down to the finer mesh details. 

Assembly loading pipeline:

Find AssemblyRoot component in scene
Request AssemblyAsset
Load in component dependencies, and any other immediately nessesary items:
Create placeholder assets for the ones that can be incrementally loaded, or are optional.
inject assembly into scene.
Start scripts.
Send out requests for remaining assets such as models and textures.
Possible event call when assembly fully loaded

[[Assets]]
[[Asset Server]]
[[Asset Manager]]