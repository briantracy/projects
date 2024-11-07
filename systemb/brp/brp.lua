-- https://www.wireshark.org/docs/wsdg_html_chunked/lua_module_Proto.html

bcp_protocol = Proto("btracy", "Brian's Protocol")

bcp_metadata = ProtoField.uint8("btracy.metadata", "Metadata", base.HEX)

local packet_types = {
    [1] = "Ack",
    [2] = "Data",
}

bcp_type = ProtoField.new("Type", "btracy.type", ftypes.UINT8, packet_types, base.DEC, 0x3, "type of packet")
bcp_seqno = ProtoField.uint8("btracy.seqno", "Sequence Number", base.DEC, nil, nil, "Sequence nunumber")
bcp_payload = ProtoField.string("btracy.payload", "Payload", base.ASCII, "BCP payload")
bcp_payload_length = ProtoField.new("Payload Length", "btracy.payload.length", ftypes.INT8)


bcp_protocol.fields = {bcp_metadata, bcp_type, bcp_payload, bcp_payload_length, bcp_seqno}

function bcp_protocol.dissector(tvbuf, pktinfo, root)
    pktinfo.cols.protocol = bcp_protocol.name;
    local pktlen = tvbuf:reported_length_remaining()
    local tree = root:add(bcp_protocol, tvbuf:range(0, pktlen))

    tree:add(bcp_type, tvbuf:range(0, 1))
    tree:add(bcp_seqno, tvbuf:range(1, 1))

    local pkt_type = tvbuf:range(0, 1):uint()
    local pkt_seqno = tvbuf:range(1, 1):uint()
    local payload_length = pktlen - 2

    -- pktinfo.cols.info:append("some text here")
    pktinfo.cols.info:set(pkt_type == 1 and "Ack " or "Data")
    pktinfo.cols.info:append(" [seqno=" .. pkt_seqno .. "]")
    if pkt_type == 2 then
        pktinfo.cols.info:append(" (payload_len=" .. payload_length .. ")")
    end


    if payload_length > 0 then
        tree:add(bcp_payload_length, payload_length):set_generated()
        tree:add(bcp_payload, tvbuf:range(2, payload_length))
    end
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(1234, bcp_protocol)
