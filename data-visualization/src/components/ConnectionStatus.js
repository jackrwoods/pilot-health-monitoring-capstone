import React, { Component } from 'react';

class ConnectionStatus extends Component {
    
  render() {
    const conFail = this.props.connection.connection ? <span style={{color: 'rgb(4, 233, 61)'}}>CON</span> : <span style={{color: 'rgb(253, 6, 11)'}}>CON FAIL</span>
    const servFail = this.props.connection.server ? <span style={{color: 'rgb(4, 233, 61)'}}>SRV</span> : <span style={{color: 'rgb(253, 6, 11)'}}>SRV FAIL</span>
    const highLat = this.props.connection.connection && this.props.ping && this.props.ping > 15 ? <span style={{color: 'rgb(253, 6, 11)'}}>LATC</span> : <span style={{color: 'rgb(4, 233, 61)'}}>LATC</span>
    return (
        <div>{conFail} | {servFail} | {highLat}</div>
    )
  }
}

export default ConnectionStatus;