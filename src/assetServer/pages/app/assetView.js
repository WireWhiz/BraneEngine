// type={} value={} index={}

//props: value={} valueChanged
class VirtualTypeEditor extends React.Component{
    constructor(props) {
        super(props);
        this.state = {

        };
        this.inputs = null;
        this.value = this.props.value.value;
    }
    onEdit(evt)
    {
        switch(this.props.value.type)
        {
            case "mat4":
            {
                let mat4 = this.inputs.map((input, index)=>{
                    try{
                        input.current.classList.remove('invalid-input');
                        return JSON.parse(input.current.value); //Using json parse because I can't get "parseFloat" to throw an error or return NaN.
                    }
                    catch(e)
                    {
                        input.current.classList.add('invalid-input');
                        return this.value[index];
                    }

                });
                this.value = mat4;
            }
                break;
            case "uintVector":
            {
                try{
                    this.inputs.current.classList.remove('invalid-input');
                    this.value = JSON.parse(this.inputs.current.value);
                }
                catch(e)
                {
                    console.log(e);
                    this.inputs.current.classList.add('invalid-input');
                    this.value = value;
                }
            }

            default:
                console.warn("input of type \"" + this.props.value.type + "\" does not yet support editing.");
        }

        this.props.onChanged(this.value);
    }

    render(){
        switch(this.props.value.type)
        {
            case "unknown":
                return "No data"
            case "mat4":
            {
                this.inputs = [];
                let rows = [];
                for (let r = 0; r < 4; r++)
                {
                    let columns = [];
                    for(let c = 0; c < 4; c++)
                    {
                        let ref = React.createRef();
                        columns.push(<input type={"text"} defaultValue={this.props.value.value[r * 4 + c]} ref={ref} onKeyUp={()=>{this.onEdit()}}/>)
                        this.inputs.push(ref);
                    }
                    rows.push(<tr>{columns}</tr>)
                }
                return <table class={"matrix"}>{rows}</table>;
            }
            case "uintVector":
            {
                this.inputs = React.createRef();
                return <input type={"text"} defaultValue={JSON.stringify(this.props.value.value)} onKeyUp={()=>{this.onEdit()}} ref={this.inputs}/>
            }

            default:
                this.inputs = React.createRef();
                return <input type={"text"} defaultValue={this.props.value.value} onKeyUp={()=>{this.onEdit()}}/>;
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

    saveAsset(){
        console.log(this.state.asset);
        fetch("/api/assets/update", {
            method: 'post',
            body: JSON.stringify(this.state.asset),
            headers:{
                'Content-Type' : 'application/json',
                'credentials' : 'same-origin'
            }
        }).then((res)=>{
            return res.json();
        }).then((json)=>{
            document.getElementById("response").innerText = json.text;
        }).catch((e)=>{
            document.getElementById("response").innerText = "client side error: " + e;
        });
    }

    removeDependency(key, index)
    {
        let newState = deepCopy(this.state);
        newState.asset.dependencies[key].splice(index, 1);
        this.setState(newState);
    }

    addDependency(key)
    {
        let newState = deepCopy(this.state);
        newState.asset.dependencies[key].push("localhost/id");
        this.setState(newState);
    }

    addEntity(){
        let newState = deepCopy(this.state);
        newState.asset.entities.push({
            components : []
        })

    }
    removeEntity(index){
        let newState = deepCopy(this.state);
        newState.asset.entities.splice(index, 1);
        this.setState(newState);
    }

    addEntityComponent(entityIndex, componentID){
        let newState = deepCopy(this.state);
        newState.asset.entities[entityIndex].push({
            id : "localhost/id",
            values : []
        });
        this.setState(newState);
    }
    removeEntityComponent(entityIndex, componentIndex)
    {
        let newState = deepCopy(this.state);
        newState.asset.entities[entityIndex].components.splice(componentIndex, 1);
        this.setState(newState);
    }

    render(){
        if(this.state.asset == null)
            this.state.asset = deepCopy(this.props.asset)
        console.log(this.state.asset)
        return[
            <h1>Assembly</h1>,
            <h2>Meshes: </h2>,
            <table class={"editable-table"}>
                <thead><th>ID</th><th>Name</th></thead>
                {this.state.asset.dependencies.meshes.map((mesh, index)=>{
                    return <tr><td>{mesh.id}</td><td>{mesh.name}</td><td><button onClick={()=>this.removeDependency("meshes", index)}>Delete</button></td></tr>
                })}
                <tfoot><td><button onClick={()=>this.addDependency("meshes")}>Add Mesh Dependency</button></td></tfoot>
            </table>,

            <h2>Entities: </h2>,
            <table class={"entity-list"}>
                <thead><th>Entity Index</th><th>Components</th></thead>
                {this.state.asset.entities.map((entity, eIndex)=> {
                    let components = entity.components.map((component, cIndex)=>{
                        return (
                            <div class={"component-view"}>
                                <p>{component.name}</p>
                                <hr/>
                                <table>
                                    {component.values.map((value, vIndex)=>{
                                        return <tr><td>{value.type}: </td><td><VirtualTypeEditor value={value} onChanged={(newValue)=>{
                                            this.state.asset.entities[eIndex].components[cIndex].values[vIndex].value = newValue
                                        }}/></td></tr>;
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
            </table>,
            <button onClick={()=>{this.saveAsset()}}>Save Asset</button>,
            <p id={"response"}></p>
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
                return <AssemblyData asset={json.data} />
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