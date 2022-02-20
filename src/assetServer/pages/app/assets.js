


class AssetList extends React.Component{
    constructor(props) {
        super(props);
    }
    assetsURL()
    {
        var showDeps = "false"

        let depsToggle = document.getElementById("showDeps");
        if(depsToggle && depsToggle.checked)
        {
            showDeps = "true";
        }
        return "/api/assets?showDeps=" + showDeps;
    }

    generateList(json)
    {
        if(!json["successful"])
        {
            return <p>Error retrieving assets</p>
        }
        if(json.assets == null)
        {
            return <p>You have no assets :(</p>
            return;
        }

        let assets = [];
        json.assets.forEach((asset)=>{
            assets.push(
                <tr onClick={() => {changePath("assets/view/" + asset.id)}} className={"asset-view-header"}>
                    <td className={"asset-id-list"}>{asset.id}</td>
                    <td>{asset.name}</td>
                    <td>{asset.type}</td>
                </tr>
            )
        });
        return assets;
    }

    render() {


        let page = [
            <h1>Assets</h1>,
            <button onClick={()=>changePath("assets/create")}><span className="material-icons-outlined">
                add_circle_outline
            </span> Create Asset</button>, <br/>,
            <label for={"showDeps"}>Show dependencies: </label>,
            <input type={"checkbox"} id={"showDeps"}/>,
            <button onClick={()=>{this.forceUpdate()}}>Update</button>,
            <table class={"asset-list"}>
                <thead>
                    <td>AssetID</td>
                    <td>Name</td>
                    <td>Type</td>
                </thead>
                <JsonAPICall src={this.assetsURL()} view1={()=>{return <p>Loading...</p>}} view2={this.generateList}/>
            </table>
        ];

        return page;
    }
}


class AssetButton extends React.Component{
    constructor(props) {
        super(props);
        this.state =
            {
            }
    }

    render() {
        return (
            <button className={"asset-button"} onClick={()=>changePath(this.props.href)}>
                <p>{this.props.name}</p>
                <p class={"material-icons-outlined"}>{this.props.icon}</p>
            </button>
        );
    }
}

class CreateAssetMenu extends React.Component{
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <div className={"asset-upload"}>
                <PathBranch pageDepth={2} basePage={[
                    <h1>Select Asset Type To Create</h1>,
                    <div class={"asset-selection"}>
                        <AssetButton href={"assets/create/gltf"} name={"Create From GLTF"} icon={"token"}/>
                    </div>
                ]} pages={{
                    "gltf" : <CreateAssetFromGLTF/>
                }} />
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
            <div class={"assets"}>
                <PathBranch pageDepth={1} basePage={<AssetList/>} pages={{
                    "create" : <CreateAssetMenu/>,
                    "view" : <AssetView/>
                }}/>
            </div>
        );
    }

}