
class CreateAssetFromGLTF extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
        }
    }

    uploadAsset(){

        let assetFile = document.getElementById("source-file").files[0]
        let formData = new FormData();

        let assetData = {
            name : document.getElementById("asset-name").value,
            extract_meshes : document.getElementById("extract-meshes").checked,
            extract_textures : document.getElementById("extract-meshes").checked
        };

        formData.append("file", assetFile);

        formData.append("assetData", new Blob([ JSON.stringify(assetData)], {type:"application/json"}));

        fetch( "/api/assets/create/gltf", {
            method: "POST",
            mode: "same-origin",
            cache: "no-cache",
            body: formData
        }).then((res)=>{
            return res.json();
        }).then((json)=>{
            document.getElementById("result").innerText = json.text;
        })

    }



    render() {
        return (
            <div class={"asset-uploading"}>
                <h1>Extract Assets From GLTF</h1>
                <label for={"assembly-name"}>Assembly Name: </label>
                <input type={"text"} id={"asset-name"} /><br/>
                <label for={"extract-meshes"}>Extract meshes: </label>
                <input type={"checkbox"} id={"extract-meshes"}/><br/>
                <label htmlFor={"extract-textures"}>Extract textures: </label>
                <input type={"checkbox"} id={"extract-textures"}/><br/>
                <label for={"file"}>Source File:</label>
                <input type={"file"} accept={".glb"} id={"source-file"} /><br/>
                <button id={"asset-file-submit"} onClick={()=>{this.uploadAsset()}}>Upload</button>
                <p id="result"></p>
            </div>
        );
    }
}