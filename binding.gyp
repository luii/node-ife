{
    'include_dirs': [
        "<!(node -e \"require('nan')\")"
    ],
    'conditions': [
        ['OS=="mac" or OS=="freebsd"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/IFE.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-bpf.cc',
                        'src/arpcache-ctlnet.cc'
                    ],
                }
            ]
        }],
        ['OS=="linux"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/IFE.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-sockpacket.cc',
                        'src/arpcache-proc.cc'
                    ],
                }
            ]
        }],
        ['OS=="solaris"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/IFE.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-dlpi.cc',
                        'src/arpcache-dlpi.cc'
                    ],
                }
            ]
        }],
        ['OS=="windows"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/IFE.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-win32.cc',
                        'src/arpcache-none.cc'
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
