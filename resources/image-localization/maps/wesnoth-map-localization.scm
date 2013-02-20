(define (pencil-effect img drawable radius strength)
    (let*
        (
            (currentlayer (car (gimp-layer-copy drawable FALSE) ) )
            (loopcounter 1)
        )

        (gimp-image-insert-layer img currentlayer 0 0)
        (gimp-hue-saturation currentlayer 0 0 0 -100)

        (set! currentlayer (car (gimp-layer-copy currentlayer FALSE) ) )
        (gimp-image-insert-layer img currentlayer 0 0)
        (plug-in-gauss-rle2 1 img currentlayer radius radius)
        (gimp-invert currentlayer)
        (gimp-layer-set-opacity currentlayer 50)

        (set! currentlayer (car (gimp-image-merge-down img currentlayer CLIP-TO-IMAGE) ) )
        (set! currentlayer (car (gimp-layer-copy currentlayer FALSE) ) )
        (gimp-image-insert-layer img currentlayer 0 0)

        (gimp-layer-set-mode currentlayer DODGE-MODE)

        (set! currentlayer (car (gimp-image-merge-down img currentlayer CLIP-TO-IMAGE) ) )

        (while (< loopcounter strength)
            (set! currentlayer (car (gimp-layer-copy currentlayer FALSE) ) )
            
            (gimp-image-insert-layer img currentlayer 0 0)

            (gimp-layer-set-mode currentlayer MULTIPLY-MODE)

            (set! loopcounter (+ loopcounter 1) )
        )

        (set! loopcounter 2)

        (while (< loopcounter strength)
            (set! currentlayer (car (gimp-image-merge-down img currentlayer CLIP-TO-IMAGE) ) )

            (gimp-layer-set-mode currentlayer MULTIPLY-MODE)

            (set! loopcounter (+ loopcounter 1) )
        )
        
        (gimp-image-remove-layer img drawable)
        (gimp-image-merge-down img currentlayer CLIP-TO-IMAGE)
    )
)

(define (wesnoth-localize-map orig_img)
    (let*
        (
            (img (car (gimp-image-duplicate orig_img) ) )
            (labels (car (gimp-image-get-layer-by-name img "labels") ) )
            (blacklabels1 0)
            (blacklabels2 0)
            (blacklabels3 0)
            (yellowlabels1 0)
            (yellowlabels2 0)
            (yellowlabels3 0)
            (labellessmap (car (gimp-image-get-layer-by-name img "labelless map") ) )
            (overlay 0)
            (overlaydiffhelper 0)
            (blurunderlay 0)
            (englishmap (car (gimp-image-get-layer-by-name img "english map") ) )
            (heightmap (car (gimp-image-get-layer-by-name img "heightmap") ) )
            (displace_amount (car (gimp-layer-get-opacity heightmap) ) )
            (deformguide (car (gimp-image-get-vectors-by-name img "deform guide") ) )
            (deformpoints 0 )
            (stroke-id 0)
        )
        
        (gimp-display-new img)

        (gimp-image-undo-group-start img)
        
        (gimp-item-set-visible labels TRUE)
        (gimp-item-set-visible labellessmap TRUE)
        (gimp-item-set-visible englishmap TRUE)
        (gimp-item-set-visible heightmap TRUE)
        (gimp-item-set-visible deformguide TRUE)
        
        (gimp-context-set-antialias FALSE)
        (gimp-context-set-feather FALSE)
        (gimp-context-set-sample-threshold-int 1)
        
        (gimp-layer-resize-to-image-size labels)
        (gimp-layer-resize-to-image-size labellessmap)
        (gimp-layer-resize-to-image-size englishmap)
        (gimp-layer-resize-to-image-size heightmap)
        
        (set! overlay (car (gimp-layer-copy labellessmap FALSE) ) )
        (gimp-image-insert-layer img overlay 0 -1)
        (gimp-item-set-visible overlay TRUE)

        (gimp-context-set-interpolation 1)
        (gimp-layer-scale heightmap 5120 3840 FALSE)
        (gimp-context-set-interpolation 0)
        (gimp-layer-scale labels 5120 3840 FALSE)
        
        (gimp-image-resize img 5120 3840 0 0)

        (set! stroke-id (aref (cadr(gimp-vectors-get-strokes deformguide) ) 0 ) )
        (set! deformpoints (caddr (gimp-vectors-stroke-get-points deformguide stroke-id) ) )

        (gimp-item-transform-perspective labels
            (* (aref deformpoints 0) 4)
            (* (aref deformpoints 1) 4)
            (* (aref deformpoints 6) 4)
            (* (aref deformpoints 7) 4)
            (* (aref deformpoints 18) 4)
            (* (aref deformpoints 19) 4)
            (* (aref deformpoints 12) 4)
            (* (aref deformpoints 13) 4)
        )
        
        (gimp-layer-resize-to-image-size labels)
        
        (plug-in-displace RUN-NONINTERACTIVE img labels 0 displace_amount FALSE TRUE heightmap heightmap 3)

        (gimp-context-set-interpolation 3)
        (gimp-layer-scale labels 1280 960 FALSE)
        
        (gimp-image-resize img 1280 960 0 0)
        
        ; separate black and yellow labels into separate layers
        (set! blacklabels1 (car (gimp-layer-copy labels FALSE) ) )
        (set! yellowlabels1 (car (gimp-layer-copy labels FALSE) ) )
        (gimp-item-set-name blacklabels1 "black labels" )
        (gimp-item-set-name yellowlabels1 "yellow labels" )
        (gimp-image-insert-layer img yellowlabels1 0 -1)
        (gimp-image-insert-layer img blacklabels1 0 -1)
        (gimp-item-set-visible blacklabels1 TRUE)
        (gimp-item-set-visible yellowlabels1 TRUE)
        
        ; erasing black labels from yellow layer and vice versa needs to be done carefully
        ; because scaling can cause some transparent pixels to change color drastically
        (gimp-context-set-sample-criterion SELECT-CRITERION-S)
        (gimp-context-set-sample-transparent FALSE)
        (gimp-context-set-sample-threshold-int 32)
        (gimp-image-select-color img CHANNEL-OP-REPLACE yellowlabels1 '(185 155 85) )
        (gimp-selection-grow img 3)
        (gimp-edit-clear blacklabels1)
        (gimp-selection-invert img)
        (gimp-edit-clear yellowlabels1)
        (gimp-context-set-sample-threshold-int 16)
        (gimp-image-select-color img CHANNEL-OP-REPLACE blacklabels1 '(0 0 0) )
        (gimp-selection-grow img 4)
        (gimp-edit-clear yellowlabels1)
        (gimp-selection-invert img)
        (gimp-edit-clear blacklabels1)
        
        (gimp-selection-none img)
        
        ; create the underlay for black labels
        (gimp-image-raise-item-to-top img overlay )
        (gimp-image-raise-item-to-top img labellessmap )
        (plug-in-gauss RUN-NONINTERACTIVE img labellessmap 15 15 1)
        (gimp-layer-set-mode labellessmap LIGHTEN-ONLY-MODE)
        (gimp-image-select-color img CHANNEL-OP-REPLACE blacklabels1 '(0 0 0) )
        (gimp-selection-grow img 2)
        (gimp-selection-invert img)
        (gimp-edit-clear labellessmap)
        (gimp-selection-none img)
        (plug-in-gauss RUN-NONINTERACTIVE img labellessmap 3 3 1)
        (set! overlay (car (gimp-image-merge-down img labellessmap CLIP-TO-IMAGE) ) )
        
        ; black labels first
        (gimp-image-raise-item-to-top img blacklabels1 )
        (set! blacklabels1 (car (pencil-effect img blacklabels1 1 20) ) )
        
        (set! blacklabels2 (car (gimp-layer-copy blacklabels1 FALSE) ) )
        (set! blacklabels3 (car (gimp-layer-copy blacklabels1 FALSE) ) )
        (gimp-item-set-name blacklabels1 "black labels")
        (gimp-item-set-name blacklabels2 "black labels 2")
        (gimp-item-set-name blacklabels3 "black labels 3")
        (gimp-layer-set-mode blacklabels1 OVERLAY-MODE)
        (gimp-layer-set-mode blacklabels2 OVERLAY-MODE)
        (gimp-layer-set-mode blacklabels3 OVERLAY-MODE)
        (gimp-image-insert-layer img blacklabels2 0 -1)
        (gimp-image-insert-layer img blacklabels3 0 -1)
        
        (gimp-image-raise-item-to-top img overlay)
        (gimp-image-raise-item-to-top img blacklabels1)
        (gimp-image-raise-item-to-top img blacklabels2)
        (gimp-image-raise-item-to-top img blacklabels3)
        (gimp-image-merge-down img blacklabels1 CLIP-TO-IMAGE)
        (gimp-image-merge-down img blacklabels2 CLIP-TO-IMAGE)
        (set! overlay (car (gimp-image-merge-down img blacklabels3 CLIP-TO-IMAGE) ) )
        
        ; yellow labels next
        (set! yellowlabels3 (car (gimp-layer-copy yellowlabels1 FALSE) ) )
        (gimp-image-insert-layer img yellowlabels3 0 -1)
        
        (gimp-layer-set-mode yellowlabels1 OVERLAY-MODE)
        (gimp-brightness-contrast yellowlabels1 -127 127)
        (plug-in-vpropagate RUN-NONINTERACTIVE img yellowlabels1 6 3 1.0 15 0 255)
        ;(plug-in-vpropagate RUN-NONINTERACTIVE img yellowlabels1 6 3 0.5 15 0 255)
        (plug-in-gauss RUN-NONINTERACTIVE img yellowlabels1 3 3 1)
        
        (set! yellowlabels2 (car (gimp-layer-copy yellowlabels1 FALSE) ) )
        (gimp-image-insert-layer img yellowlabels2 0 -1)
        
        (gimp-item-set-name yellowlabels1 "yellow labels")
        (gimp-item-set-name yellowlabels2 "yellow labels 2")
        (gimp-item-set-name yellowlabels3 "yellow labels 3")
        
        (gimp-image-raise-item-to-top img yellowlabels1 )
        (gimp-image-raise-item-to-top img yellowlabels2 )
        (gimp-image-raise-item-to-top img yellowlabels3 )
        (gimp-image-merge-down img yellowlabels1 CLIP-TO-IMAGE)
        (gimp-image-merge-down img yellowlabels2 CLIP-TO-IMAGE)
        (set! overlay (car (gimp-image-merge-down img yellowlabels3 CLIP-TO-IMAGE) ) )

        ; combining everything with the labelless map (onto which the localized labels have now been merged)
        ; (gimp-image-raise-item-to-top img labellessmap )
        (set! overlaydiffhelper (car (gimp-layer-copy overlay FALSE) ) )
        (gimp-image-insert-layer img overlaydiffhelper 0 0)
        
        ; creating an image diff
        (gimp-image-raise-item-to-top img englishmap)
        (gimp-image-raise-item-to-top img overlaydiffhelper)
        
        (gimp-layer-set-mode overlaydiffhelper DIFFERENCE-MODE)
        (gimp-layer-flatten overlaydiffhelper)
        (gimp-layer-flatten englishmap)

        (set! englishmap (car (gimp-image-merge-down img overlaydiffhelper CLIP-TO-IMAGE) ) )
        
        (gimp-context-set-sample-criterion SELECT-CRITERION-COMPOSITE)
        (gimp-context-set-sample-threshold-int 1)
        (gimp-image-select-color img CHANNEL-OP-REPLACE englishmap '(0 0 0) )
        
        (gimp-edit-clear overlay)
        (gimp-item-set-name overlay "localized overlay")
        (gimp-selection-none img)
        
        ; deleting everything else but the final overlay
        (gimp-image-remove-layer img labels)
        (gimp-image-remove-layer img englishmap)
        (gimp-image-remove-layer img (car (gimp-image-get-layer-by-name img "label placement guide") ) )
        (gimp-image-remove-layer img heightmap)
        (gimp-image-remove-vectors img deformguide)
        
        (gimp-image-undo-group-end img)
        (gimp-displays-flush)
    )
)

(script-fu-register
    "wesnoth-localize-map"
    "Map Localization"
    "Creates a localized label overlay"
    "zookeeper"
    "public domain"
    ""
    "RGBA"
    SF-IMAGE    "Image"    0
)

(script-fu-menu-register "wesnoth-localize-map" "<Image>/Filters/Wesnoth")
