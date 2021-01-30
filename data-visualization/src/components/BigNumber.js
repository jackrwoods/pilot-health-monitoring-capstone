import React, { Component } from 'react';
import './BigNumber.css';

class Graph extends Component {
  render() {
    return (
        <span class="BigNumber">{this.props.children} {this.props.uom}</span>
    )
  }
}



export default Graph;