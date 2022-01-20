class AssetList extends React.Component{
    constructor(props) {
        super(props);
    }

    getAssets(){
        //Worry about this later

    }

    render() {
        return [
            <h1>Assets</h1>,
            <button onClick={()=>changePath("assets/create")}><span className="material-icons-outlined">
                add_circle_outline
            </span> Create Asset</button>
        ];
    }
}

class CreateAsset extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            assetType : "mesh"
        }
    }

    selectAssetType(){
        let assetType = document.getElementById("asset-type").value;
        this.setState({
            assetType : assetType
        })
    }

    uploadAsset(){
        let assetType = this.state.assetType;

        let formData = new FormData();
        let assetData = {
            name : document.getElementById("asset-name").value,
            type : assetType
        };


        switch(assetType)
        {
            case "mesh":
            {
                let assetFile = document.getElementById("asset-file").files[0]
                formData.append("file", assetFile);
                break;
            }
            case "texture":
            {
                let assetFile = document.getElementById("asset-file").files[0]
                formData.append("file", assetFile);
                break;
            }
            case "scene":
            {

            }
        }

        formData.append("assetData", new Blob([ JSON.stringify(assetData)], {type:"application/json"}));

        fetch( "/api/create-asset", {
            method: "POST",
            mode: "same-origin",
            cache: "no-cache",
            body: formData
        });

    }

    displayTypeOption(){
        let assetType = this.state.assetType;
        switch(assetType){
            case "mesh":
                return [<input type={"file"} id={"asset-file"} />,<br/>];
            case "texture":
                return [<input type={"file"} id={"asset-file"} />,<br/>];
            case "scene":
                return ;

            default:
                this.setState({
                    assetType : "mesh"
                })
        }
    }

    render() {
        return (
            <div class={"asset-uploading"}>
                <h1>Create Asset</h1>
                <label for={"asset-name"}>Asset Name: </label>
                <input type={"text"} id={"asset-name"} /><br/>
                <label htmlFor={"asset-type"}>Asset Type: </label>
                <select id={"asset-type"} onChange={()=>this.selectAssetType()}>
                    <option value={"mesh"}>Mesh</option>
                    <option value={"texture"}>Texture</option>
                    <option value={"scene"}>Scene</option>
                </select><br/>
                {this.displayTypeOption()}
                <br/>
                <button id={"asset-file-submit"} onClick={()=>{this.uploadAsset()}}>Upload</button>
            </div>
        );
    }
}

class Assets extends React.Component{
    constructor(props) {
        super(props);
    }

    displayPage(){
        var path = currentPath();
        if(path.length < 2)
            return <AssetList/>;
        switch(path[1]){
            case "create":
                return <CreateAsset/>
            default:
                changePath("assets")
        }
    }

    render() {

        return (
            <div class={"assets"}>
                {this.displayPage()}
            </div>
        );
    }

}