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

class AssetButton extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
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