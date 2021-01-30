import React, { Component } from 'react';
import './WarningPane.css';

class WarningPane extends Component {
  render() {
    return (
        <div className="WarningPane">
            {this.props.children}
        </div>
    )
  }
}

export default WarningPane;