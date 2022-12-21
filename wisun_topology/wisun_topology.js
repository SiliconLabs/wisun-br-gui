const address_label_length = 4

const network_chart = document.getElementById("network_chart")
const infobox_arrow_button = document.getElementById("infobox_arrow_button")
const infobox = document.getElementById("infobox")
const autofit = document.getElementById("autofit")
const node_header = document.getElementById('node_header')
const node_eui64 = document.getElementById('node_eui64')
const node_parent = document.getElementById('parent_eui64')
const node_ipv6 = document.getElementById('node_ipv6')
const node_children = document.getElementById('node_children')
const node_count = document.getElementById('node_count')
const wsbrd_connect_error = document.getElementById('wsbrd_connect_error')

let wisun
let nodes_last = []

let check_dbus_interval
let explore_network_interval
let explore_running = true
let network_nodes
let network_edges

network_nodes = new vis.DataSet()
network_edges = new vis.DataSet()

let data = {
  nodes: network_nodes,
  edges: network_edges
}

let options = {
  width: '100%',
  height: '100%',
  physics: {
    stabilization: false,
  },
  layout: {
    improvedLayout: true
  }
}  

let wisun_network = new vis.Network(network_chart, data, options)

window.addEventListener('focus', explore_network_start)
window.addEventListener('blur', explore_network_stop)

init()

function explore_network_start() {
  if (!explore_running) {
    explore_network_interval = window.setInterval(explore_network_change, 1000);
  }
}

function explore_network_stop() {
  window.clearInterval(explore_network_interval)
  explore_running = false
}

function bin_to_text(bin) {
  let text = ""
  for (let cnt = 0; cnt < bin.length; cnt++) {
    text += ("0" + bin.charCodeAt(cnt).toString(16)).slice(-2).toUpperCase()
  }
  return text
}

function address_to_label(address) {
  return bin_to_text(address).slice(-address_label_length)
}

function eui64_add_colon(eui64) {
  eui64_colon = eui64.slice(0, 2)
  for (octet = 1; octet < eui64.length / 2; octet++) {
    eui64_colon += ":" + eui64.slice(octet * 2, octet * 2 + 2)
  }
  return eui64_colon
}

function ipv6_add_colon(ipv6) {
  eui64_colon = ipv6.slice(0, 4)
  for (octet = 1; octet < ipv6.length / 4; octet++) {
    eui64_colon += ":" + ipv6.slice(octet * 4, octet * 4 + 4)
  }
  return eui64_colon
}
function update_info_box() {
  if (wisun_network.getSelectedNodes().length == 1) {
    node_children.innerHTML = nodes_last.filter(node => node.parent_address == wisun_network.getSelectedNodes()[0]).length
    node_ipv6.innerHTML = ipv6_add_colon(bin_to_text(nodes_last.filter(node => node.node_address == wisun_network.getSelectedNodes()[0])[0].node_ipv6)).replace(/\b:?(?:0+:?){2,}/, '::')
    // check if the node exist in the node_last array - the only node which is not in the list must be the border router
    if (nodes_last.filter(node => node.node_address == wisun_network.getSelectedNodes()[0]).length == 1) {
      parent_address = eui64_add_colon(bin_to_text(nodes_last.filter(node => node.node_address == wisun_network.getSelectedNodes()[0])[0].parent_address))
    } else {
      parent_address = "N/A"
    }
    parent_eui64.innerHTML = parent_address
  } else {
    node_children.innerHTML = ""
    node_ipv6.innerHTML = ""
    parent_eui64.innerHTML = ""
  }
}

function explore_network_change() {
  let properties = wisun.proxy()
  properties.wait(function () {
    if(properties.valid) {
      if (properties.Nodes.length < 1) {
        return
      }
      let nodes_new = []
      for(cnt = 0; cnt < properties.Nodes.length; cnt++) {
        if(typeof(properties.Nodes[cnt][1].is_border_router) != 'undefined') {
          nodes_new.push({
            node_address: atob(properties.Nodes[cnt][0]),
            node_ipv6: atob(properties.Nodes[cnt][1]['ipv6']['v'][1]),
            parent_address: 0
          })
          
        } else if(typeof(properties.Nodes[cnt][1].parent) != 'undefined' && properties.Nodes[cnt][1]['ipv6']['v'].length == 2){
        nodes_new.push({
            node_address: atob(properties.Nodes[cnt][0]),
            node_ipv6: atob(properties.Nodes[cnt][1]['ipv6']['v'][1]),
            parent_address: atob(properties.Nodes[cnt][1]['parent']['v'])
          })
        }
      }

      // check if there are new items in the nodes compared to the
      // previous iteration that have to be added to the network
      let nodes_joined = nodes_new.filter(function(item_new) {
        return !(nodes_last.filter(function(item_last) {
          return item_last.node_address == item_new.node_address
        }).length)
      })

      // check if there are additional items in the node list of the
      // previous iteration compared the current query that have to
      // be removed from the network
      let nodes_left = nodes_last.filter(function(item_last) {
        return !(nodes_new.filter(function(item_new) {
          return item_new.node_address == item_last.node_address
        }).length)
      })

      // check if there are nodes which moved to another parent
      // since the last query that need to be updated
      let nodes_updated = nodes_new.filter(function(item_new) {
        return (nodes_last.filter(function(item_last) {
          return item_last.node_address == item_new.node_address && item_last.parent_address != item_new.parent_address
        }).length)
      })

      for (let cnt = 0; cnt < nodes_joined.length; cnt++) {
        let node_color;
        if (nodes_joined[cnt].parent_address != 0) {
          node_color = "#cfffcf"
        } else {
          node_color = "#ffcfcf"
        }
        network_nodes.add({
          id: nodes_joined[cnt].node_address,
          node_ipv6: nodes_joined[cnt].node_ipv6,
          parent_address: nodes_joined[cnt].parent_address,
          label: address_to_label(nodes_joined[cnt].node_address),
          color: node_color,
          x: 0,
          y: 0})
        
        if(nodes_joined[cnt].parent_address != 0) {
          network_edges.add({
            id: nodes_joined[cnt].node_address,
            from: nodes_joined[cnt].node_address,
            to: nodes_joined[cnt].parent_address,
            color: {color: "#30af30", highlight: '#30af30'}})
        }
      }

      for (let cnt = 0; cnt < nodes_left.length; cnt++) {
        network_nodes.remove(nodes_left[cnt].node_address)
        network_edges.remove(nodes_left[cnt].node_address)
      }

      for (let cnt = 0; cnt < nodes_updated.length; cnt++) {
        let node_color;
        if (nodes_updated[cnt].parent_address != 0) {
          node_color = "#cfffcf"
        } else {
          node_color = "#ffcfcf"
        }
        network_nodes.update({
          id: nodes_updated[cnt].node_address,
          node_ipv6: nodes_updated[cnt].node_ipv6,
          parent_address: nodes_updated[cnt].parent_address,
          label: address_to_label(nodes_updated[cnt].node_address),
          color: node_color,
          x: 0,
          y: 0})
        if(nodes_updated[cnt].parent_address != 0) {
          network_edges.update({
            id: nodes_updated[cnt].node_address,
            from: nodes_updated[cnt].node_address,
            to: nodes_updated[cnt].parent_address,
            color: {color: "#30af30", highlight: '#30af30'}})
        }
      }
      nodes_last = nodes_new.slice()
      if(autofit.checked) {
        zoom_to_fit()
      }
      node_count.innerHTML = "Node count: " + nodes_last.length
      update_info_box()
    }
  })
}

function check_dbus_properties() {
  properties = wisun.proxy()
  properties.wait(function () {
    if(properties.valid) {
      if (typeof(properties.Nodes) == 'undefined') {
        return
      } else {
        clearInterval(check_dbus_interval)
        wsbrd_connect_error.style.display = "none"
        explore_network_interval = setInterval(explore_network_change, 1000)
        explore_running = true
     }
    }
  })
}

function init() {
  wisun = cockpit.dbus("com.silabs.Wisun.BorderRouter", {"bus": "system"})
  properties = wisun.proxy()
  properties.wait(function () {
    if(properties.valid) {
      // if wsbrd is just started properties can be retrieved properties like HwAddress or Nodes may not be available
      // to avoid relying undefined values an interval timer function is started that checks the existencny of the value
      check_dbus_interval = setInterval(check_dbus_properties, 1000)
    }
  })
}

function zoom_to_fit() {
  let options = {
    offset: {x: 0, y: 0},
    duration: 1000,
    easingFunction: "linear",
  }
  wisun_network.fit({ animation: options })
}

infobox_arrow_button.onclick = function() {
  if(infobox.style.display == 'none') {
    infobox.style.display = 'block'
    infobox_arrow_button.classList.remove("fa-angle-double-left")
    infobox_arrow_button.classList.add("fa-angle-double-right")
  } else {
    infobox.style.display = 'none'
    infobox_arrow_button.classList.remove("fa-angle-double-right")
    infobox_arrow_button.classList.add("fa-angle-double-left")
  }
}

wisun_network.on('selectNode', function(params) {
  node_eui64.innerHTML = eui64_add_colon(bin_to_text((network_nodes.get(params.nodes))[0].id))
  update_info_box()
})

let previous_node

wisun_network.on('dragging', function(params) {
  if (typeof((network_nodes.get(params.nodes))[0]) == 'undefined') {
    return
  }
  if(wisun_network.getSelectedNodes().length == 1) {
    if(previous_node != network_nodes.get(params.nodes)[0].id) {
      wisun_network.selectNodes(wisun_network.getSelectedNodes())
      node_eui64.innerHTML = eui64_add_colon(bin_to_text((network_nodes.get(params.nodes))[0].id))
      update_info_box()
      previous_node = network_nodes.get(params.nodes)[0].id
    }
  }
})

wisun_network.on('deselectNode', function(params) {
  node_header.innerHTML = "Device"
  node_eui64.innerHTML = ""
  parent_eui64.innerHTML = ""
  update_info_box()
})

cockpit.transport.wait(function() { })
