# Shell routine to check QUERY_STRING args.

# Check vars are safe before we set them. 
for f in `echo "$QUERY_STRING" | tr '&' ' '`; do
    case "$f" in
	W_*)
	    if echo "$f" | grep -qv '^[A-Za-z0-9_]*=[A-Za-z0-9_+.,-]*$'; then exit 1; fi
	    ;;
	*)
	    exit 1
	    ;;
    esac
done
eval `echo "$QUERY_STRING" | tr '&' ' '`
