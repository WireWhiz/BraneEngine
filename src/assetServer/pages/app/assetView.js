// type={} value={} index={}

class AssetIDName extends  React.Component{

    render(){
        return (<JsonAPICall src={"/api/assets/get_name"} headers={{"name" : this.props.assetID}} view1={()=>{return this.props.assetID}} view2={(json)=>{return json.name}}/>)
    }
}

//props: value={} onEdit={}
class VirtualTypeEditor extends React.Component{
    render(){
        switch(this.props.value.type)
        {
            case "unknown":
                return "No data"
            case "mat4":
            {
                let comps = [];
                for (let i = 0; i < 16; i++)
                {
                    comps.push(<input type={"text"} defaultValue={this.props.value.value[i]}/>)
                    if((i + 1) % 4 === 0 && i !== 0)
                        comps.push(<br/>);
                }
                return comps;
            }

            default:
                return <input type={"text"} defaultValue={this.props.value.value} />;
        }
    }
}

//props: asset={}
class AssemblyData extends React.Component
{
    constructor(props) {
        super(props);
        this.state = {
            asset : null
        };
    }

    removeDependency(key, index)
    {
        let newState = deepCopy(this.state);
        newState.asset.data.dependencies[key].splice(index, 1);
        this.setState(newState);
    }

    addDependency(key)
    {
        let newState = deepCopy(this.state);
        newState.asset.data.dependencies[key].push("localhost/id");
        this.setState(newState);
    }

    addEntity(){
        let newState = deepCopy(this.state);
        newState.asset.data.entities.push({
            components : []
        })

    }
    removeEntity(index){
        let newState = deepCopy(this.state);
        newState.asset.data.entities.splice(index, 1);
        this.setState(newState);
    }

    addEntityComponent(entityIndex, componentID){
        let newState = deepCopy(this.state);
        newState.asset.data.entities[entityIndex].push({
            id : "localhost/id",
            values : []
        });
        this.setState(newState);
    }
    removeEntityComponent(entityIndex, componentIndex)
    {
        let newState = deepCopy(this.state);
        newState.asset.data.entities[entityIndex].components.splice(componentIndex, 1);
        this.setState(newState);
    }

    render(){
        if(this.state.asset == null)
            this.state.asset = deepCopy(this.props.asset)
        console.log(this.state.asset)
        return[
            <h2>Meshes: </h2>,
            <table class={"editable-table"}>
                <thead><th>AssetID</th></thead>
                {this.state.asset.data.dependencies.meshes.map((id, index)=>{
                    return <tr><td><AssetIDName assetID={id}/></td><td><button onClick={()=>this.removeDependency("meshes", index)}>Delete</button></td></tr>
                })}
                <tfoot><td><button onClick={()=>this.addDependency("meshes")}>Add Mesh Dependency</button></td></tfoot>
            </table>,

            <h2>Entities: </h2>,
            <table class={"entity-list"}>
                <thead><th>Entity Index</th><th>Components</th></thead>
                {this.state.asset.data.entities.map((entity, eIndex)=> {
                    let components = entity.components.map((component, cIndex)=>{
                        return (
                            <div class={"component-view"}>
                                <p><AssetIDName assetID={component.id}/></p>
                                <hr/>
                                <table>
                                    {component.values.map((value)=>{
                                        return <tr><td>{value.type}: </td><td><VirtualTypeEditor value={value}/></td></tr>;
                                    })}
                                </table>
                            </div>
                        )

                    });

                    return (
                        <tr>
                            <td valign={"top"}>{eIndex}</td>
                            <td>
                                {components}
                                <div className={"component-view"}>
                                    <input type={"text"} placeholder={"componentID"}/><button>AddComponent</button>
                                </div>
                            </td>
                        </tr>
                    );
                })}
                <tfoot><button onClick={()=>this.addEntity()}>Add Entity</button></tfoot>
            </table>
        ]
    }

}

class AssetData extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            data : null
        };
    }

    renderData(json){
        switch(json.type) {
            case "assembly":
            {
                return <AssemblyData asset={json} />
            }
            default:
                return <p>No data preview for this type.</p>
        }
    }

    render() {
        return (
          <div class={"asset-data"}>
              <h2>Asset Data: </h2>
              <JsonAPICall src={"/api/assets/" + this.props.assetID + "/data"} view1={()=><a>Loading...</a>} view2={(json)=> this.renderData(json)}/>
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
                    <AssetData type={this.props.type} assetID={this.props.assetID}/>,
                    //<AssetDependencies assetID={this.props.assetID} />
                ];
            }} />
        ];
    }
}