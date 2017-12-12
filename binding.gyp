{
    'conditions': [
        ['OS=="linux"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'addon.cc',
                        'ife.cc',
                        'ife-icmp-support.cc',
                        'ife-sockpacket.cc',
                        'arpcache-proc.cc'
                    ],
                }
            ]
        }],
        ['1==1', {
            'targets': [ 
                {
                    'target_name': 'IFEStub',
                    'type': 'none'
                }
            ]
        }]
    ]
}
