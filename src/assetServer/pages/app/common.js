
const deepCopy = (object) => {
    let newObject

    if (typeof object !== "object" || object === null) {
        return object
    }

    newObject = Array.isArray(object) ? [] : {}

    for (let key in object) {
        newObject[key] = deepCopy(object[key])
    }

    return newObject
}

class Expandable extends React.Component{
    constructor(props) {
        super(props);
    }

    render(){
        return this.props.expanded ? this.props.view2() : this.props.view1();
    }
}

class JsonAPICall extends React.Component
{
    constructor(props) {
        super(props);
        this.state = {
            json : null,
            src : ""
        }
    }

    apiCall()
    {
        let headers = this.props.headers;
        if(headers == null)
            headers = {};
        headers.credentials = 'same-origin';
        fetch(this.props.src, {
            headers : headers
        }).then((res) =>{
            if(res.ok)
                return res.json();
            else
                return null;
        }).then((json) =>{
            this.setState({
                json : json,
                src : this.props.src
            });
        });
    }

    render(){
        if(this.state.json == null || this.props.src != this.state.src)
        {
            this.apiCall();
            return this.props.view1()
        }

        return this.props.view2(this.state.json);
    }
}

class PathBranch extends React.Component{
    render()
    {
        let path = currentPath();
        if(path.length <= this.props.pageDepth)
            return this.props.basePage;
        let page = path[this.props.pageDepth]
        if (page in this.props.pages)
            return this.props.pages[page];
        else
            return this.props.basePage;

    }
}

//Props index={} elements={} checkedItems={} onEdit={}
class EditableTableRow extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            editing : false,
            inputRefs : []
        }

    }

    doneEditing(){
        let newElements = [];
        this.state.inputRefs.forEach((input)=>{
            newElements.push(input.current.value);
        })
        this.props.onEdit(this.props.index, newElements);
        this.setState({editing:false, inputRefs: []})
    }

    render()
    {
        if(this.props.noEdit || !this.state.editing){
            let itemElements = [<td valign={"top"}><input type={"checkbox"} onClick={(event)=>{
                let index = this.props.index;
                if(event.target.checked)
                    this.props.checkedItems[index] = event.target;
                else
                    delete this.props.checkedItems[index];
            }} /></td>];
            this.props.elements.forEach((element)=>{
                itemElements.push(<td>{element}</td>)
            });
            if(!this.props.noEdit)
                itemElements.push(
                    <td valign={"bottom"}><button onClick={()=>this.setState({editing:true, inputRefs: []})}>edit</button></td>
                )
            return <tr class={"editable-table-item"}>{itemElements}</tr>;
        }
        else
        {
            let itemElements = [<td></td>];
            this.props.elements.forEach((element)=>{
                this.state.inputRefs.push(React.createRef());
                itemElements.push(<td><input type={"text"} defaultValue={element} ref={this.state.inputRefs[this.state.inputRefs.length - 1]}/></td>)
            });
            itemElements.push(
                <td valign={"bottom"}><button onClick={()=>{this.doneEditing();}}>done</button></td>
            )
            return <tr class={"editable-table-item"}>{itemElements}</tr>;
        }
    }
}

//Props list={} headerElements={} itemElements={function to render list item} createItem={returns new item} listChanged={changed(newList)} itemChanged={}
class EditableTable extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            checkedItems : {},
            items : []
        }
    }
    render()
    {
        let headerItems = []
        this.props.headerElements.forEach((item)=>{
            headerItems.push(<th>{item}</th>);
        });
        this.state.elements = [];
        let itemIndex = 0;
        let noEdit = this.props.noEdit != null && this.props.noEdit;
        this.props.list.forEach((item)=>{
            this.state.elements.push(<EditableTableRow index={itemIndex} elements={this.props.itemElements(itemIndex++, item)} checkedItems={this.state.checkedItems}
            onEdit={(index, newElements)=>{
                this.props.itemChanged(index, newElements);
            }} noEdit={noEdit}/>);
        });
        return (
            <table class={"editable-table"}>
                <thead>
                    <td></td> {/*Empty column for checkbox*/}
                    {headerItems}
                </thead>
                {this.state.elements}
                <tfoot>
                    <td colSpan={4}>
                        <button onClick={()=>{
                            let index = 0;
                            console.log(this.state.checkedItems);
                            let newItems =  this.props.list.filter((item)=>{
                                let checked = index in this.state.checkedItems;
                                if(checked)
                                    this.state.checkedItems[index].checked =false;
                                index++;
                                return !checked;

                            })
                            this.props.listChanged(newItems);

                        }}>Delete Selected</button>
                        <button onClick={()=>{
                            let newItem = this.props.createItem();
                            this.props.listChanged(this.props.list.concat([newItem]));
                        }}>Add New</button>
                    </td>
                </tfoot>
            </table>);
    }
}