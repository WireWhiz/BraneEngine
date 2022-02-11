class AssetThumbnail extends React.Component{
    constructor(props){
        super(props);
    }
    render() {
        return (
            <button className={"asset-thumbnail"} onClick={()=>{changePath("assets/" + this.props.assetID); console.log(this.props.assetID)}}>
                <p>{this.props.name}</p>
                <p class={"material-icons-outlined"}>{this.props.icon}</p>
            </button>
        );
    }
}

class AssetList extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            assetsLoaded : false,
            assets : null
        }
    }

    getAssets(){
        fetch("/api/assets").then((res) =>{
            return res.json();
        }).then((json) =>{
            if(!json["successful"])
            {
                this.setState({
                    assetsLoaded : true,
                    assets : <p>Error retrieving assets</p>
                });
                return;
            }
            if(json.assets == null)
            {
                this.setState({
                    assetsLoaded : true,
                    assets : <p>You have no assets :(</p>
                });
                return;
            }

            var assets = [];
            json.assets.forEach((asset)=>{
                assets.push(
                    <AssetThumbnail name={asset.name} icon={"token"} assetID={asset.id}/>
                )
            });
            this.setState({
                assetsLoaded : true,
                assets : assets
            });
        });

    }

    render() {
        if(!this.state.assetsLoaded)
            this.getAssets();
        let page = [
            <h1>Assets</h1>,
            <button onClick={()=>changePath("assets/create")}><span className="material-icons-outlined">
                add_circle_outline
            </span> Create Asset</button>,
            <div class={"asset-list"}>
                {(this.state.assetsLoaded) ? this.state.assets : <p>Loading...</p>}
            </div>
        ];

        this.state.assetsLoaded = false;

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
        this.state = {
        }
    }

    displayPage(){
        var path = currentPath();
        if(path.length < 3)
            return [
                <h1>Select Asset Type To Create</h1>,
                <div class={"asset-selection"}>
                    <AssetButton href={"assets/create/gltf"} name={"Create From GLTF"} icon={"token"}/>
                </div>
                ];
        switch(path[2]){
            case "gltf":
                return <CreateAssetFromGLTF/>
            default:
                changePath("assets/create")
        }
    }

    render() {
        return (
            <div className={"asset-upload"}>
                {this.displayPage()}
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
                return <CreateAssetMenu/>
            default:
                if(path.length > 2){
                    changePath("assets")
                    return;
                }
                return <AssetView assetID={path[1]}/>
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