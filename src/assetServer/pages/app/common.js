

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
        fetch(this.props.src).then((res) =>{
            return res.json();
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