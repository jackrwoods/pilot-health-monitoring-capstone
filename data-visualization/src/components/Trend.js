import React, { Component } from 'react';

class Trend extends Component {
  render() {
    const stress = true ? <span style={{color: 'rgb(4, 233, 61)'}}>UNSTR</span> : <span style={{color: 'rgb(253, 6, 11)'}}>STRSD</span>
    

    const len = this.props.data.HR.length;
    let trend = <span style={{color: 'rgb(4, 233, 61)'}}>TRND —</span>
    if (len > 2000) {
        let rawData = this.props.data.HR;
        let slope = (rawData.slice(len / 2, len - 1).reduce((a, b) => a + b) - rawData.slice(0, len / 2).reduce((a, b) => a + b))
        if (Math.abs(slope) > 0.2) {
            if (slope > 0) {
                trend = <span style={{color: 'rgb(252, 220, 39)'}}>TRND ▲</span>
            } else {
                trend = <span style={{color: 'rgb(4, 233, 61)'}}>TRND ▼</span>
            }
        }
    }

    return (
        <span>{stress} | {trend}</span>
    )
  }
}

export default Trend;