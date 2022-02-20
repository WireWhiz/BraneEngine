class DependencyListItem extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
        }
    }

    render(){
        return (
            <tr contentEditable={true}>
                <td><input type={"checkbox"} className={"dep-select"} ref={this.props.checkboxRef} defaultChecked={false}/></td>
                <td className={"asset-id-list"}>{this.props.assetID}</td>
                <td>{this.props.level}</td>
                <td>{this.props.type}</td>
            </tr>);
    }

}

class AssetDependencies extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            dependencies : []
        }
    }

    uploadDependencies(){
        console.log("Uploading")
    }

    render(){
        return (
            <JsonAPICall src={"/api/assets/" + this.props.assetID + "/dependencies"} view1={()=>{return <p>Loading...</p>;}} view2={(json)=>{
                if(!json.successful)
                    return <p>Was unable to load dependencies</p>;
                if(json.dependencies != null && this.state.dependencies.length === 0)
                    json.dependencies.forEach((dep)=>{
                        this.state.dependencies.push(
                            <DependencyListItem assetID={dep.id} level={dep.level} type={dep.type} checkboxRef={React.createRef()}/>
                        );
                    });

                return (
                <div class={"dependencies-list"}>
                    <h2>Dependencies: </h2>
                    <table class={"asset-list"}>
                        <thead>
                            <td></td>
                            <td>AssetID</td>
                            <td>Level</td>
                            <td>Type</td>
                        </thead>
                        {this.state.dependencies}
                        <tfoot>
                            <td colSpan={4}>
                                <button onClick={()=>{
                                    let newDeps = [];
                                    newDeps = this.state.dependencies.filter((item)=>{
                                        if(item.props.checkboxRef.current.checked)
                                        {
                                            item.props.checkboxRef.current.checked = false;
                                            return false;
                                        }
                                        return true;

                                    })
                                    this.setState({
                                        dependencies: newDeps
                                    });
                                }}>Delete Selected</button>
                                <button onClick={()=>{
                                    let newDeps = this.state.dependencies.concat([<DependencyListItem assetID={"domain/id"} level={"1"} type={"none"} checkboxRef={React.createRef()}/>])
                                    this.setState({
                                        dependencies:newDeps
                                    });
                                }}>Add New</button>
                                <button onClick={()=>{
                                    this.setState({
                                        dependencies : []
                                    })
                                }}>Reset</button>
                                <button onClick={this.uploadDependencies}>Apply Changes</button>
                            </td>
                        </tfoot>
                    </table>
                </div>);
            }} />
        );
    }
}

class EntityView extends React.Component{
    constructor(props) {
        super(props);
    }

    render(){
        return
    }
}

class AssetData extends React.Component{
    constructor(props) {
        super(props);
    }

    renderData(json){
        let types = {
            assembly : [
                //Grab all entities from json and display them along with their components.
            ]
        }
        if(this.props.type in types)
            return types[this.props.type]
        return <p>No data preview for this type.</p>
    }

    render() {
        return (
          <div class={"asset-data"}>
              <h2>Asset Data: </h2>
              <JsonAPICall src={"api/assets/" + this.props.assetID + "/data"} view1={<a>Loading...</a>} view2={this.renderData}/>
          </div>
        );
    }
};

class AssetView extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
        }
    }

    render()
    {
        if(this.props.assetID == null)
        {
            let path = currentPath();
            this.props.assetID = path[path.length - 1]; // See if the last element is the id
        }
        return [
            <JsonAPICall src={"/api/assets/" + this.props.assetID} view1={()=>{return <p>Loading...</p>;}} view2={(json)=>{
                if(!json.successful)
                    return <p>Was unable to load asset</p>;
                return [
                    <h1>{json.name}</h1>,
                    <p>Viewing Asset: {json.name}<br/>
                    AssetID: {this.props.assetID}</p>,
                    <assetData type={this.props.type} assetID={this.props.assetID}/>,
                    <AssetDependencies assetID={this.props.assetID} />
                ];
            }} />
        ];
    }
}