class UploadAsset extends React.Component{
    constructor(props) {
        super(props);
    }

    uploadAsset(){
        let assetFile = document.getElementById("asset-file").files[0]
        let formData = new FormData();

        formData.append("json", new Blob([JSON.stringify({name:"test asset"})], {type:"application/json"}), "info.json");
        formData.append("file", assetFile);

        fetch("/api/set-asset-source",{
                method: "POST",
                mode: "same-origin",
                cache: "no-cache",
                body: formData
            });

    }

    render() {
        return (
            <div class={"asset-uploading"}>
                <input type={"file"} id={"asset-file"} />
                <button id={"asset-file-submit"} onClick={()=>{this.uploadAsset()}}>Upload</button>
            </div>

        );
    }
}

class Assets extends React.Component{
    constructor(props) {
        super(props);
    }
    render() {
        return (
            <div class={"profile"}>
                <h1>Assets</h1>
                <p class={"username"}></p>
                <UploadAsset />

            </div>
        );
    }

}